#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "absl/strings/numbers.h"
#include "absl/algorithm/container.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "absl/strings/str_split.h"
#include "rtc_base/network.h"
#include "rtc_base/ip_address.h"
#include "rtc_base/synchronization/sequence_checker.h"

#include "api/array_view.h"
#include "config.h"
#include "gateway.h"
#include "atomic.h"
#include "handle/nice_handle.h"
#include "handle/p2p_handle.h"
#include "jwt/jwt.h"
#include "auth.h"
#include "plugins/p2p_call.h"

static std::unique_ptr<Gateway> gateway_ = nullptr;
static std::unique_ptr<rtc::Thread> main_thread = nullptr;

static gateway_cfg* config_ = NULL;

static volatile int stop = 0;
static int stop_signal = 0;

//using absl::numbers_internal;
static char *config_file = NULL;
static char *configs_folder = NULL;

/* Daemonization */
static bool daemonize = false;
static int pipefd[2];

/* Public instance name */
static const char *server_name = NULL;

#define DEFAULT_CONFDIR  "."

static std::string public_ip;
static std::string local_ip;

static std::unique_ptr<webrtc::TaskQueueFactory> task_queue_factory_ = 
		webrtc::CreateDefaultTaskQueueFactory();

/* The default timeout for sessions is 60 seconds: this means that, if
 * we don't get any activity (i.e., no request) on this session for more
 * than 60 seconds, then it's considered expired and we destroy it. That's
 * why we have a keep-alive method in the API. This can be overridden in
 * either janus.cfg/.jcfg or from the command line. Setting this to 0 will
 * disable the timeout mechanism, which is NOT suggested as it may risk
 * having orphaned sessions (sessions not controlled by any transport
 * and never freed). Besides, notice that if you make this shorter than
 * 30s, you'll have to update the timers in janus.js when the long
 * polling mechanism is used and shorten them as well, or you'll risk
 * incurring in unexpected timeouts (when HTTP is used in janus.js, the
 * long poll is used as a keepalive mechanism). */
#define DEFAULT_SESSION_TIMEOUT		60
static int session_timeout = DEFAULT_SESSION_TIMEOUT;

#define DEFAULT_RECLAIM_SESSION_TIMEOUT		0
static int reclaim_session_timeout = DEFAULT_RECLAIM_SESSION_TIMEOUT;

/*event 2 second check it*/
#define DEFAULT_SESSION_CHECK_DELAY		1000 * 2
static const int session_check_delay = DEFAULT_SESSION_CHECK_DELAY;

//static gateway_request exit_request;
#define GATE_WAY_REQUEST_THREAD_NUMBER 12;

static bool accept_new_sessions = true;

enum {
	MSG_CHECK_SESSION = 0,
	MSG_CLOSE_SESSION = 1,	
};

static gateway_json_parameter incoming_request_parameters[] = {
	{"transaction", json_string, GATEWAY_JSON_PARAM_REQUIRED | GATEWAY_JSON_PARAM_NONEMPTY},
	{"gateway", json_string, GATEWAY_JSON_PARAM_REQUIRED},
	{"id", json_uint, GATEWAY_JSON_PARAM_POSITIVE}
};

static gateway_json_parameter create_session_parameters[] = {
	{"session_uid", json_string, GATEWAY_JSON_PARAM_REQUIRED | GATEWAY_JSON_PARAM_NONEMPTY}
};

static gateway_json_parameter attach_parameters[] = {
	{"plugin", json_string, GATEWAY_JSON_PARAM_REQUIRED},
	{"opaque_id", json_string, 0},
};

class Gateway::LogFileSink : public rtc::LogSink {
 public:
 	/**
 	 * file full path
 	 */
  explicit LogFileSink(const std::string& logfile) 
  	: logfile_(NULL),logFile(logfile) {}
	
  virtual ~LogFileSink() {
		if (logfile_ != NULL)
			fclose(logfile_);
		logfile_ = NULL;
	}
	
	inline size_t Size() {
		size_t size = 0;
		if (logfile_ != NULL) {
			fseek(logfile_, 0L, SEEK_END);
			size = ftell(logfile_);
		}
		return size;
	}
	
  inline void Start() {
  	#define MAX_LOG_FILE_SIZE (1024*1024)
		if (NULL == logfile_)
			logfile_ = fopen(logFile.c_str(), "w");
		else if (Size() > MAX_LOG_FILE_SIZE) {
			Close();
			logfile_ = fopen(logFile.c_str(), "w");
		}
	}
	
  inline void Close() {
		if(logfile_ != NULL) {
			fclose(logfile_);
			logfile_ = NULL;
		}
	}
	
  virtual void OnLogMessage(const std::string& message) {
		rtc::CritScope lock(&logLock);
		Start();
		if (NULL == logfile_)
			return;
		if(fwrite(message.c_str(), 1, message.length(), logfile_) < 0) {
			Close();
		} else if(fflush(logfile_) < 0) {
			Close();
		}
	}
 private:
  FILE* logfile_;
  std::string logFile;
  rtc::CriticalSection logLock;
};

class Gateway::RequestTask : public webrtc::QueuedTask {
	public:
		explicit RequestTask(
			Gateway* gateway,gateway_request request) 
				: gateway_(gateway),request_(request) {}
		~RequestTask() = default;
		bool Run();
	private:
		Gateway* gateway_;
		gateway_request request_;
};

bool Gateway::RequestTask::Run()
{
	if (atomic_int_get(&stop))
		return true;
	if (!gateway_ || !request_ || request_->message == NULL)
		return true;
  Json::Reader reader;
  Json::Value obj;
  if (!reader.parse(request_->message, obj)) {
		std::string error = "Received unknown message. ";
		error.append(request_->message);
    RTC_LOG(WARNING) << error;
    return true;
  }
	
	std::string gateway_value;
	rtc::GetStringFromJsonObject(obj, "gateway",&gateway_value);
	if(!gateway_value.empty() 
		&& !strcasecmp(gateway_value.c_str(), 
			request_->admin ? "message_plugin" : "message")) {
		gateway_->thread_pool()->Schedule(
			std::bind(&Gateway::requestsProcess, std::move(request_)));
	} else {
		if(!request_->admin)
			gateway_->requestProcess(std::move(request_));
		else
			gateway_->requestAdminProcess(std::move(request_));
	} 
	return true;
}

Gateway::Gateway()	
	: clock_(webrtc::Clock::GetRealTimeClock()),
	  request_queue_(task_queue_factory_->CreateTaskQueue(
			"request_worker_queue",
			webrtc::TaskQueueFactory::Priority::NORMAL))			
{
	RTC_LOG(INFO) << "current time:" << rtc::TimeUTCMillis();
	parseCommonConfig();
	parseNiceConfig();	
	createPlugins();
	std::unique_ptr<ITransport> transport = 
			createTransport(TRANSPORT_WEBSOCKET,this);	
	rtc::CritScope lock(&transportsLock); 
	transports_.insert(
		std::pair<TransportType,std::unique_ptr<ITransport>>
		(transport->getType(),std::move(transport))
	);
	thread_pool_ = rtc::CreateThreadPool(12);
	transports_[TRANSPORT_WEBSOCKET]->connect(configs_folder);
	session_watchdog_ = rtc::Thread::Create();
	session_watchdog_->SetName("session watch dog", nullptr);	
	RTC_CHECK(session_watchdog_->Start()) << "Failed to start thread";	
  session_watchdog_->PostDelayed(RTC_FROM_HERE, session_check_delay, this, MSG_CHECK_SESSION/*,0*/);
	notify_thread_ = rtc::Thread::Create();
	notify_thread_->SetName("notify thread", nullptr);
	RTC_CHECK(notify_thread_->Start()) << "Failed to start thread";	
}

Gateway::~Gateway()
{
	
	RTC_LOG(INFO) << __FUNCTION__;	
	gateway_auth_deinit();
	session_watchdog_->Stop();
	session_watchdog_.reset();
	destorySessions();
	destoryPlugins();
	thread_pool_.reset();
	transports_.clear();
}

/**
 * Callback to check with the core if an API secret must be provided
 */
bool Gateway::IsSecretNeeded(gateway_transport* transport)
{
	return false;
}
/**
 * Callback to check with the core if an API secret valid
 */ 
bool Gateway::IsSecretValid(
	gateway_transport* transport,const std::string& secret)
{
	return true;
}
/**
 * Callback to check with the core if an authtoken nedded
 */ 
bool Gateway::IsAuthTokenNeeded(gateway_transport* transport)
{
	return false;
}
/**
 * Callback to check with the core if an authtoken valid
 */ 	
bool Gateway::IsAuthTokenValid(
	gateway_transport* transport,const std::string& auth_token) 
{	
	return true;
}

/**
 * request
 */
void Gateway::IncomingRequest(
	gateway_transport* transport,
	gateway_transport_session& session,
	void *request_id, bool admin,	char* message)
{
	rtc::scoped_refptr<GatewayRequest> request_ = 
				gatewayRequestNew(transport, session, request_id,admin,message);
	std::unique_ptr<webrtc::QueuedTask> task = 
				absl::make_unique<RequestTask>(this,std::move(request_));
	request_queue_.PostTask(std::move(task));
}
/**
 * gone
 */
void Gateway::TransportGone(
	gateway_transport* transport,
	gateway_transport_session& session)
{
	rtc::StringBuilder stringBuilder;
	stringBuilder.AppendFormat("A %s transport instance has gone away (%p)",transport->getPackage().c_str(), session);
	RTC_LOG(INFO) << "Start :" << __FUNCTION__ << " " << stringBuilder.str();
	stringBuilder.Clear();
	rtc::CritScope lock(&sessionsLock);
	if (sessions_.empty()) return;

	for(auto iterator=sessions_.begin();
		iterator != sessions_.end(); iterator++) {
		auto& session_ = iterator->second;
		auto key = iterator->first;
		if(session_ == nullptr 
					|| atomic_int_get(&session_->destroyed) 
					|| atomic_int_get(&session_->timeout) 
					|| session_->last_activity == 0)
			continue;	
		if(session_->source && session_->source->instance == session) {
			stringBuilder.AppendFormat("  -- Session %lld will be over if not reclaimed",session_->session_id);
			RTC_LOG(INFO) << stringBuilder.str();
			stringBuilder.Clear();
			stringBuilder.AppendFormat("  -- Marking Session %lld as over",session_->session_id);
			RTC_LOG(INFO) << stringBuilder.str();
			stringBuilder.Clear();
			if(reclaim_session_timeout < 1) { /* Reclaim session timeouts are disabled */
				/* Mark the session as destroyed */
				gatewaySessionDestory(session_);	
				uid_sessions.erase(session_->session_uid);
				sessions_.erase(iterator);
			} else {
				/* Set flag for transport_gone. The Janus sessions watchdog will clean this up if not reclaimed */
				atomic_int_set(&session_->transport_gone, 1);
			}
		}		
	}
}

/**
 * @param session: gateway session
 * @param event : jsson event message
 */
void Gateway::NotifyEvent(
	gateway_transport* transport,
	gateway_transport_session& session,
	const std::string& message) 
{
	RTC_LOG(INFO) << "Start :" << __FUNCTION__ << " " << message;
}

int Gateway::sendEvent(
	const gateway_plugin& plugin,
	const gateway_plugin_session& session,
	const std::string& transaction,
	const json_value& message,
	const json_value& jsep)
{
	return 0;
}

void Gateway::colsePc(const gateway_plugin_session& session)
{
	if (session == nullptr || session->g_handle == nullptr)
		return;
	/* A plugin asked to get rid of a PeerConnection: enqueue it as a timed source */
	if(!session->g_handle->sessionIsAlive(session))
		return;
	const gateway_handle& handle = session->g_handle;
	if(!atomic_int_compare_and_exchange(&handle->closepc, 0, 1))
		return;
}

void Gateway::destorySessions() 
{
	rtc::CritScope lock(&sessionsLock);
	if (sessions_.empty()) return;

	for(auto iterator = sessions_.begin();
		iterator != sessions_.end(); iterator++) {
		auto& session = iterator->second;
		if(session == nullptr || atomic_int_get(&session->destroyed))
			continue; 
		gatewaySessionClearHandles(session);
		/* Notify the transport */
		const gateway_request& source = gatewaySessionRequest(session);
		if(source) {
			Json::Value event(
				createGatewayMessage("down", session->session_id, std::string()));
			Json::StyledWriter writer;	
			/* Send this to the transport client and notify the session's over */
			source->transport->sendMessage(source->instance, NULL, false, writer.write(event));
			source->transport->sessionOver(source->instance, session->session_id, true, false);
		}		
		gatewaySessionDestory(session);	
		
		uid_sessions.erase(session->session_uid);
		/* FIXME Is this safe? apparently it causes hash table errors on the console */
		sessions_.erase(iterator);
	}
	sessions_.clear();	
}

void Gateway::checkSessions() 
{
	RTC_DCHECK_RUN_ON(session_watchdog_.get()); 
	
	/* Session timeouts are disabled */
	if(session_timeout < 1) {
		session_watchdog_->PostDelayed(RTC_FROM_HERE, 
			session_check_delay, this, MSG_CHECK_SESSION/*,0*/);
		return ;
	}	
	
	//rtc::CritScope lock(&sessionsLock);
	sessionsLock.Enter();
	if (sessions_.empty()) {
		sessionsLock.Leave();
		session_watchdog_->PostDelayed(RTC_FROM_HERE, 
			session_check_delay, this, MSG_CHECK_SESSION/*,0*/);
		return ;
	}
	
	for(auto iterator = sessions_.begin();
		iterator != sessions_.end(); iterator++) {
		auto& session = iterator->second;
		if(session == nullptr || atomic_int_get(&session->destroyed))
			continue; 
		int64_t now = rtc::SystemTimeMillis();
		if ( (now - session->last_activity >= (int64_t)session_timeout * rtc::kNumMillisecsPerSec 
						&& !atomic_int_compare_and_exchange(&session->timeout, 0, 1)) 
		  || ((atomic_int_get(&session->transport_gone) 
		  			&& 	now - session->last_activity >= (int64_t)reclaim_session_timeout * rtc::kNumMillisecsPerSec) 
		  			&& !atomic_int_compare_and_exchange(&session->timeout, 0, 1)))	 {
			RTC_LOG(INFO) << "Timeout expired for session :" << session->session_id;
			/* Mark the session as over, we'll deal with it later */
			gatewaySessionClearHandles(session);
			/* Notify the transport */
			const gateway_request& source = gatewaySessionRequest(session);
			if(source) {
				Json::Value event(
					createGatewayMessage("timeout", session->session_id, std::string()));
				Json::StyledWriter writer;	
				/* Send this to the transport client and notify the session's over */
				source->transport->sendMessage(source->instance, NULL, false, writer.write(event));
				source->transport->sessionOver(source->instance, session->session_id, true, false);
			}
			#if 0
			/* Notify event handlers as well */
			if(janus_events_is_enabled())
				janus_events_notify_handlers(JANUS_EVENT_TYPE_SESSION, JANUS_EVENT_SUBTYPE_NONE,
					session->session_id, "timeout", NULL);
			#endif			
			gatewaySessionDestory(session);	
			uid_sessions.erase(session->session_uid);
			/* FIXME Is this safe? apparently it causes hash table errors on the console */
			sessions_.erase(iterator);
		}	
	}
	sessionsLock.Leave();
	
	session_watchdog_->PostDelayed(RTC_FROM_HERE, 
		session_check_delay, this, MSG_CHECK_SESSION/*,0*/);
}

void Gateway::OnMessage(rtc::Message* message) 
{
	int message_id = message->message_id;	
	switch (message_id) {
		case MSG_CHECK_SESSION: {
			checkSessions();
			break;
		}
		case MSG_CLOSE_SESSION: {
			checkSessions();
			break;
		}		
		default:
			break;
	}
}

gateway_request Gateway::gatewayRequestNew(
	gateway_transport* transport, 
	gateway_transport_session& instance, 
	void *request_id, bool admin,char* message) 
{

	rtc::scoped_refptr<GatewayRequest> request = 
					new rtc::RefCountedObject<GatewayRequest>();	
	request->gateway_p = (gateway*)this;	
	request->transport = transport;
	request->instance = instance;
	request->request_id = request_id;
	request->admin = admin;
	request->message = message;
	atomic_int_set(&request->destroyed, 0);
	return request;
}

void Gateway::gatewayRequestDestory(gateway_request& request) 
{
	if(request == nullptr || 
			!atomic_int_compare_and_exchange(&request->destroyed, 0, 1))
		return;
	/* This request can be destroyed, free all the resources */
	request->gateway_p = NULL;
	request->transport = NULL;
	request->request_id = NULL;
	request->instance = nullptr;
	if (request->message != NULL) {
		free(request->message);
	}
	request->message = NULL;
	//request->Release();
	request = nullptr;
}

uint64_t Gateway::gatewaySessionIdFind(const std::string& session_uid) 
{
	rtc::CritScope lock(&sessionsLock);
	if (session_uid.empty() || uid_sessions.empty()) 
		return 0;	
	auto iterator = uid_sessions.find(session_uid);
	if (iterator != uid_sessions.end()) 
		return iterator->second; 
	return 0;	
}

gateway_session Gateway::gatewaySessionFind(uint64_t session_id) 
{
	rtc::CritScope lock(&sessionsLock);

	if (sessions_.empty() || session_id == 0) 
		return nullptr;
	
	auto iterator = sessions_.find(session_id);
	if (iterator != sessions_.end())
		return iterator->second;
	return nullptr;
}

gateway_session Gateway::gatewaySessionCreate(
	uint64_t session_id,const std::string& session_uid) 
{
	if (session_uid.empty()) return nullptr;
	
	if(session_id == 0) {
		while(session_id == 0) {
			session_id = gateway_random_uint64();
			gateway_session session = gatewaySessionFind(session_id);
			if(session != nullptr) {
				/* Session ID already taken, try another one */
				session_id = 0;
			}				
		}
	}
	
	gateway_session session = 
					new rtc::RefCountedObject<GatewaySession>();
	RTC_LOG(INFO) << "Creating new session: " << session_id;
	session->gateway_p = (gateway*)this;	
	session->session_id = session_id;
	session->session_uid = session_uid;
	session->source = NULL;
	atomic_int_set(&session->destroyed, 0);
	atomic_int_set(&session->timeout, 0);
	atomic_int_set(&session->transport_gone, 0);
	session->last_activity = rtc::SystemTimeMillis();
	//session->session_handles = NULL;

	rtc::CritScope lock(&sessionsLock);
	sessions_.insert(
		std::pair<uint64_t,gateway_session>
		(session->session_id,session)
	);

	uid_sessions.insert(
		std::pair<std::string,uint64_t>
		(session_uid,session->session_id)
	);

	return session;
}

/* Destroys a session but does not remove it from the sessions hash table. */
int Gateway::gatewaySessionDestory(gateway_session& session) 
{
	uint64_t session_id = session->session_id;
	RTC_LOG(INFO) << "Destroying session: " << session_id;
	if(!atomic_int_compare_and_exchange(&session->destroyed, 0, 1))
		return 0;
	gatewaySessionClearHandles(session);
	if(session->source != nullptr) {
		gatewayRequestDestory(session->source);
		session->source = nullptr;
	}
	session->gateway_p = NULL;
	return 0;
}

gateway_request Gateway::gatewaySessionRequest(gateway_session& session) 
{
	if(session == nullptr)
		return nullptr;
	rtc::CritScope lock(&session->mutex);
	gateway_request source = session->source;
	if(source == nullptr 
			|| atomic_int_get(&source->destroyed)) {
		source == nullptr;
	} 
	return source;
}

gateway_handle Gateway::gatewaySessionFindHandles(gateway_session& session, uint64_t handle_id) 
{
	if(session == nullptr || session->gateway_p == NULL)
		return nullptr;
	gateway_handle handle = nullptr;
	rtc::CritScope lock(&session->mutex);
	if (!session->session_handles.empty()) {
		auto iterator = session->session_handles.find(handle_id);
		if (iterator != session->session_handles.end()) {
			handle = iterator->second;
		} 
	}
	return handle;
}

void Gateway::gatewaySessionInsertHandles(gateway_session& session, gateway_handle& handle) 
{
	if(session == nullptr || 
		session->gateway_p == NULL || handle == nullptr)
		return;
	rtc::CritScope lock(&session->mutex);
	session->session_handles.insert(
		std::pair<uint64_t,gateway_handle>
		(handle->handle_id,handle)
	);
}

int Gateway::gatewaySessionRemoveHandles(gateway_session& session, gateway_handle& handle)
{
	rtc::CritScope lock(&session->mutex);
	int error = handle->destorySession();
	auto iterator = 
		session->session_handles.find(handle->handle_id);
	if (iterator != session->session_handles.end()) {
		iterator->second = nullptr;
		session->session_handles.erase(iterator);
	}
	return error;
}

void Gateway::gatewaySessionClearHandles(gateway_session& session) 
{
	rtc::CritScope lock(&session->mutex);
	if (session->session_handles.empty()) return;
	for(auto iterator=session->session_handles.begin();
		iterator != session->session_handles.end(); iterator++) {
		if (iterator->second != nullptr) {
			iterator->second->destorySession();
			iterator->second = nullptr;
			//gateway_ice_handle_destroy(session, handle);	
		}
		session->session_handles.erase(iterator);
	}	
}

gateway_plugin Gateway::gatewayPluginFind(const std::string& plugin)
{
	if (plugin.empty()) return nullptr;
	gateway_plugin plugin_ = nullptr;
	rtc::CritScope lock(&pluginsLock);
	if (!plugins_.empty()) {
		auto iterator = plugins_.find(plugin);
		if (iterator != plugins_.end()) {
			plugin_ = iterator->second;
		} 
	}
	return plugin_;	
}


json_value Gateway::createGatewayMessage(
	const std::string& status, uint64_t session_id, const std::string& transaction) 
{
	Json::Value msg;
	msg["gateway"] = status;
	if(session_id > 0)
		msg["session_id"] = session_id;
	if(!transaction.empty())
		msg["transaction"] = transaction;
	return std::move(msg);
}

json_value Gateway::gatewayInfo(const std::string& transaction)
{
	Json::Value info(createGatewayMessage("server_info",0,transaction));
	info["name"] = "gateway";	
	info["version"] = 1.0;
	info["version_string"] = "1.0";
	info["author"] = "Jeffrey@ubonass.com";
	return std::move(info);
}

int Gateway::requestProcessSuccess(
		gateway_request& request,const Json::Value& payload)
{
	if(request == nullptr || payload.empty())
		return -1;
	/* Pass to the right transport plugin */
	RTC_LOG(LS_VERBOSE) << "Sending " << (request->admin ? "admin" : "gateway") 
											<< "  API response to " << request->transport->getPackage();
	Json::StyledWriter writer;	
	return request->transport->sendMessage(
						request->instance, 
						request->request_id, 
						request->admin, 
						writer.write(payload));
}


int Gateway::requestProcessErrorString(
	gateway_request& request, uint64_t session_id, const std::string& transaction, int error, char* error_string)
{
	if(request == nullptr)
		return -1;
	
	/* Done preparing error */
	RTC_LOG(LS_VERBOSE) << transaction << " Returning " << (request->admin ? "admin" : "Gateway")
											<< " API error " << error << " (" << error_string << ")";
	/* Prepare JSON error */
	Json::Value reply(createGatewayMessage("error", session_id, transaction));
	Json::Value error_data;
	error_data["code"] = error;
	error_data["reason"] = error_string;
	reply["error"] = error_data;
	/* Pass to the right transport plugin */
	Json::StyledWriter writer;
	return request->transport->sendMessage(
						request->instance, 
						request->request_id, 
						request->admin, 
						writer.write(reply));
}

int Gateway::requestProcessError(
	gateway_request& request, uint64_t session_id, const std::string& transaction, int error, const char *format, ...)
{
	if(request == nullptr )
		return -1;
	
	char *error_string = NULL;
	char error_buf[512];
	if(format == NULL) {
		/* No error string provided, use the default one */
		error_string = (char *)gateway_api_error(error);
	} else {
		/* This callback has variable arguments (error string) */
		va_list ap;
		va_start(ap, format);
		vsnprintf(error_buf, sizeof(error_buf), format, ap);
		va_end(ap);
		error_string = error_buf;
	}
	return requestProcessErrorString(request, session_id, transaction, error, error_string);
}


void Gateway::requestsProcess(gateway_request request)
{
	if(request == nullptr || request->gateway_p) {
		RTC_LOG(INFO) << "Missing request or gateway";
		return;
	}
	gateway* gateway_p = (gateway*)request->gateway_p;
	if(!request->admin)
		gateway_p->requestProcess(request);
	else
		gateway_p->requestAdminProcess(request);
	/* Done */
	gateway_p->gatewayRequestDestory(request);	
}

int Gateway::requestBaseProcess(gateway_request& request,const Json::Value& root)
{
	int ret = -1;	
	if (request == nullptr || root.empty())
		return ret;	
	
	//RTC_LOG(INFO) << "start " << __FUNCTION__ 
	//							<< " message:" << request->message;
	std::string session_uid;
	uint64_t session_id = 0;	
	std::string transaction;
	std::string gateway_type;

	int error_code = 0;
	char error_cause[100];		

	
	rtc::GetStringFromJsonObject(root, "gateway",&gateway_type);
	
	if (root.isMember("transaction"))
		rtc::GetStringFromJsonObject(root, "transaction",&transaction);
	
	//rtc::GetUInt64FromJsonObject(root, "session_id",&session_id);	
	/* Can only be a 'Create new session', a 'Get info' or a 'Ping/Pong' request */
	if(!strcasecmp(gateway_type.c_str(), "info")) {
		Json::Value reply(gatewayInfo(transaction));
		ret = requestProcessSuccess(request, reply);
		return ret;
	}
	
	if(!strcasecmp(gateway_type.c_str(), "ping")) {
		/* Prepare JSON reply */
		Json::Value reply(createGatewayMessage("pong", 0, transaction));
		ret = requestProcessSuccess(request, reply);
		return ret;
	}
		
	if(strcasecmp(gateway_type.c_str(), "create")) {
		ret = requestProcessError(
				request, session_id, transaction, 
				GATEWAY_ERROR_INVALID_REQUEST_PATH, "Unhandled request '%s' at this path", gateway_type);
		return ret;
	}
	
	/* Make sure we're accepting new sessions */
	if(!accept_new_sessions) {
		ret = requestProcessError(
				request, session_id, transaction, 
				GATEWAY_ERROR_NOT_ACCEPTING_SESSIONS, NULL);
		return ret;
	}

	/* Any secret/token to check? */		
	if(gateway_auth_is_enabled()) {
		std::string token;
		rtc::GetStringFromJsonObject(root, "token",&token);
		if (token.empty())
			return requestProcessError(request, 
					session_id, transaction, GATEWAY_ERROR_UNAUTHORIZED, NULL);
		if (!gateway_auth_check_token(token.c_str(),NULL,NULL))		
			return requestProcessError(request, 
					session_id, transaction, GATEWAY_ERROR_UNAUTHORIZED, NULL);
	}

	/* Get transaction and message request */
	GATEWAY_VALIDATE_JSON_OBJECT(root, create_session_parameters,
		error_code, error_cause, false,
		GATEWAY_ERROR_MISSING_MANDATORY_ELEMENT, GATEWAY_ERROR_INVALID_ELEMENT_TYPE);
	if(error_code != 0) {
		ret = requestProcessError(request, session_id, transaction, error_code, error_cause);
		return ret;
	}
	gateway_session session;
	/*make sure public id is uniqueness*/
	rtc::GetStringFromJsonObject(root, "session_uid",&session_uid);
	
	session_id = 0;
	if (root.isMember("id"))
		rtc::GetUInt64FromJsonObject(root, "id",&session_id);

	if (gatewaySessionIdFind(session_uid) > 0)
		return requestProcessError(request, session_id, transaction, 
					GATEWAY_ERROR_SESSION_CONFLICT, "Session has already in use");
	
	if(session_id != 0) 
		/* The application provided the session ID to use */
		if(session_id > 0 && (session = gatewaySessionFind(session_id)) != NULL) 
			/* Session ID already taken */
			return requestProcessError(request, session_id, transaction, 
					GATEWAY_ERROR_SESSION_CONFLICT, "Session ID already in use");
		
	
	
	/* Handle it */
	session = gatewaySessionCreate(session_id,session_uid);
	if(session == nullptr) {
		ret = requestProcessError(
				request, session_id, transaction, 
				GATEWAY_ERROR_UNKNOWN, "Memory error");
		return ret;
	}
	session_id = session->session_id;
	/* We increase the counter as this request is using the session */
	//gateway_refcount_increase(&session->ref);
	/* Take note of the request source that originated this session (HTTP, WebSockets, RabbitMQ?) */
	session->source = 
			gatewayRequestNew(request->transport, request->instance, NULL, false, NULL);
	/* Notify the source that a new session has been created */
	request->transport->sessionCreated(request->instance, session->session_id);
#if 0	
	/* Notify event handlers */
	if(janus_events_is_enabled()) {
		/* Session created, add info on the transport that originated it */
		json_t *transport = json_object();
		json_object_set_new(transport, "transport", json_string(session->source->transport->get_package()));
		char id[32];
		memset(id, 0, sizeof(id));
		g_snprintf(id, sizeof(id), "%p", session->source->instance);
		json_object_set_new(transport, "id", json_string(id));
		janus_events_notify_handlers(JANUS_EVENT_TYPE_SESSION, JANUS_EVENT_SUBTYPE_NONE,
			session_id, "created", transport);
	}
#endif	
	/* Prepare JSON reply */
	Json::Value reply(createGatewayMessage("success", 0, transaction));
	Json::Value data;
	data["id"] = session_id;
	reply["data"] = data;
	/* Send the success reply */
	return requestProcessSuccess(request, reply);
}


int Gateway::requestAttachProcess(gateway_session& session,gateway_request& request,const Json::Value& root)
{
	int ret = -1;	
	if (session == nullptr 
			|| request == nullptr || root.empty())
		return ret;	
	
	int error_code = 0;
	char error_cause[100];	
	uint64_t session_id = 0;
	std::string transaction;
	std::string plugin;
	std::string opaque_id;
	std::string media_type;
	rtc::StringBuilder stringBuilder;
	
	rtc::GetStringFromJsonObject(root, "transaction",&transaction);
	rtc::GetUInt64FromJsonObject(root, "session_id",&session_id);
	
	GATEWAY_VALIDATE_JSON_OBJECT(root, attach_parameters,
		error_code, error_cause, FALSE,
		GATEWAY_ERROR_MISSING_MANDATORY_ELEMENT, GATEWAY_ERROR_INVALID_ELEMENT_TYPE);
	if(error_code != 0) {
		ret = requestProcessError(request, session_id, transaction, error_code, error_cause);
		return ret;
	}	
	
	rtc::GetStringFromJsonObject(root, "plugin",&plugin);	
	const gateway_plugin& plugin_ = gatewayPluginFind(plugin);
	if(plugin_ == nullptr) {
		ret = requestProcessError(request, session_id, transaction, GATEWAY_ERROR_PLUGIN_NOT_FOUND, "No such plugin '%s'", plugin.c_str());
		return ret;
	}
	
	/* If the auth token mechanism is enabled, we should check if this token can access this plugin */
	if(gateway_auth_is_enabled()) {
		std::string token;
		rtc::GetStringFromJsonObject(root, "token",&token);
		if (token.empty())
			return requestProcessError(request, 
					session_id, transaction, GATEWAY_ERROR_UNAUTHORIZED_PLUGIN, NULL);

		if (!gateway_auth_check_contains(token.c_str(),"plugin",plugin_->getName().c_str()))		
			return requestProcessError(request, 
					session_id, transaction, GATEWAY_ERROR_UNAUTHORIZED_PLUGIN, NULL);
	}
	if (root.isMember("opaque_id")) 
		rtc::GetStringFromJsonObject(root, "opaque_id",&opaque_id);
	if (root.isMember("media_type"))
		rtc::GetStringFromJsonObject(root, "media_type",&media_type);	
	else 
		media_type = "p2p";//p2p or relay	

	Json::Value ask; 
	if (root.isMember("ask") && 
			rtc::GetValueFromJsonObject(root,"ask",&ask)) {
		std::string type;	
		std::vector<std::string> peerNames;			
		if (!ask.isMember("type"))
			return requestProcessError(request, session_id, transaction, 
							GATEWAY_ERROR_ASK_TYPE_NOT_FOUND, NULL);
		if (!rtc::GetStringFromJsonObject(ask,"type",&type) || type.empty())
			return requestProcessError(request, session_id, transaction,
							GATEWAY_ERROR_ASK_TYPE_EMPTY, NULL);
		if (strcasecmp(type.c_str(),"call") 
					&& strcasecmp(type.c_str(),"join"))
			return requestProcessError(request, session_id, transaction,
							GATEWAY_ERROR_ASK_TYPE_ERROR, NULL);
		Json::Value array = ask["data"];
		if (!array.isArray() 
				|| !rtc::JsonArrayToStringVector(array,&peerNames) 
				|| peerNames.size() <= 0)
				return requestProcessError(request, session_id, transaction,
					GATEWAY_ERROR_ASK_PARAMS_ERROR, NULL);	
		
		notify_thread_->Invoke<void>(RTC_FROM_HERE, [&] {
				RTC_DCHECK_RUN_ON(notify_thread_.get());
	
			for(int i= 0; i < peerNames.size(); i++) {
				gateway_session peer = 
					gatewaySessionFind(gatewaySessionIdFind(peerNames[i]));	
				/* Notify the transport */
				const gateway_request& source = gatewaySessionRequest(peer);
				if(source) {
					Json::Value msg;
					msg["gateway"] = "ask";
					msg["type"] = type;
					msg["media_type"] = media_type;
					msg["peer"] = session->session_uid;
					Json::StyledWriter writer;	
					/* Send this to the transport client and notify the session's over */
					source->transport->sendMessage(source->instance, NULL, false, writer.write(msg));
				}				
			}	
		});	
	}
	
	gateway_handle handle;	
	/* Create handle */
	if(!strcasecmp(media_type.c_str(), "relay")) 
		handle = new rtc::RefCountedObject<NiceHandle>(session,opaque_id);
	 else 
		handle = new rtc::RefCountedObject<P2pHandle>(session,opaque_id);
	if (handle == nullptr) {
		ret = requestProcessError(request, session_id, transaction, GATEWAY_ERROR_UNKNOWN, "Memory error");
		return ret;
	}
	
	gatewaySessionInsertHandles(session, handle);
	/* Attach to the plugin */
	int error = 0;
	if((error = handle->createSession(plugin_)) != 0) {
		/* TODO Make error struct to pass verbose information */
		gatewaySessionRemoveHandles(session, handle);
		stringBuilder.AppendFormat("Couldn't attach to plugin '%s', error '%d'",plugin,error);
		RTC_LOG(LERROR) << stringBuilder.str();
		stringBuilder.Clear();
		ret = requestProcessError(request, session_id, transaction, GATEWAY_ERROR_PLUGIN_ATTACH, "Couldn't attach to plugin: error '%d'", error);
		return ret;
	}
	/* Prepare JSON reply */
	Json::Value reply(createGatewayMessage("success", session_id, transaction));
	Json::Value data;
	data["id"] = handle->handle_id;
	reply["data"] = data;
	/* Send the success reply */
	return requestProcessSuccess(request, reply);	
}

int Gateway::requestProcess(gateway_request request)
{
	int ret = -1;	
	if (request == nullptr || request->message == NULL)
		return ret;

	Json::Reader reader;
  Json::Value root;
  if (!reader.parse(request->message, root)) {
		std::string error = "Received unknown message. ";
		error.append(request->message);
    RTC_LOG(WARNING) << error;
    return ret;
  }
	
	int error_code = 0;
	char error_cause[100];		
	uint64_t session_id = 0, handle_id = 0;
	rtc::StringBuilder stringBuilder;
	
	RTC_LOG(INFO) << "start " << __FUNCTION__ 
								<< " root:" << rtc::JsonValueToString(root);	
	
	if (root.isMember("session_id"))
		rtc::GetUInt64FromJsonObject(root, "session_id",&session_id);

	if (root.isMember("handle_id"))
		rtc::GetUInt64FromJsonObject(root, "handle_id",&handle_id);

	/* Get transaction and message request */
	GATEWAY_VALIDATE_JSON_OBJECT(root, incoming_request_parameters,
		error_code, error_cause, false,
		GATEWAY_ERROR_MISSING_MANDATORY_ELEMENT, GATEWAY_ERROR_INVALID_ELEMENT_TYPE);
	if(error_code != 0) {
		ret = requestProcessError(request, session_id, std::string(), error_code, error_cause);
		return ret;
	}
	
	gateway_session session = nullptr;
	gateway_handle handle = nullptr;
	
	std::string transaction;
	std::string gateway_type;
	
	rtc::GetStringFromJsonObject(root, "transaction",&transaction);
	
	rtc::GetStringFromJsonObject(root, "gateway",&gateway_type);	

	if(session_id == 0 && handle_id == 0)
		return requestBaseProcess(request,root);

	if(session_id < 1) {
		RTC_LOG(LERROR) << "Invalid session";
		return requestProcessError(request, session_id, transaction, GATEWAY_ERROR_SESSION_NOT_FOUND, NULL);
	}
	
	if(root.isMember("handle_id") && handle_id < 1) {
		RTC_LOG(INFO) << "Invalid handle";
		return requestProcessError(request, session_id, transaction, GATEWAY_ERROR_SESSION_NOT_FOUND, NULL);
	}
#if 0		
	/* Go on with the processing */
	ret = gateway_request_check_secret(request, session_id, transaction);
	if(ret != 0) {
		ret = requestProcessError(request, session_id, transaction, GATEWAY_ERROR_UNAUTHORIZED, NULL);
		goto done;
	}
#endif
	/* If we got here, make sure we have a session (and/or a handle) */
	session = gatewaySessionFind(session_id);
	if(!session) {
		RTC_LOG(LERROR) << "Couldn't find any session " << session_id;
		return requestProcessError(request, session_id, transaction, GATEWAY_ERROR_SESSION_NOT_FOUND, "No such session %lld", session_id);
	}	
	/* Update the last activity timer */
	session->last_activity = rtc::SystemTimeMillis();
	handle = nullptr;
	if(handle_id > 0) {
		handle = gatewaySessionFindHandles(session, handle_id);
		if(!handle) {
			stringBuilder.AppendFormat("Couldn't find any handle %lld in session %lld...",handle_id, session_id);
			RTC_LOG(INFO) << stringBuilder.str();
			stringBuilder.Clear();
			return requestProcessError(request, session_id, transaction, GATEWAY_ERROR_HANDLE_NOT_FOUND, "No such handle %lld in session %lld", handle_id, session_id);
		}
	}	
	GatewayType type = gatewayTypeFromString(gateway_type.c_str());
	switch(type) {
		case GATEWAY_KEEPALIVE: {
			/* Just a keep-alive message, reply with an ack */
			Json::Value reply(createGatewayMessage("ack", session_id, transaction));
			/* Send the success reply */
			return requestProcessSuccess(request, reply);			
		}
		
		case GATEWAY_ASK:	
		case GATEWAY_ATTACH: {
			if(handle != NULL) {
				/* Attach is a session-level command */
				return requestProcessError(request, session_id, transaction, GATEWAY_ERROR_INVALID_REQUEST_PATH, "Unhandled request '%s' at this path", gateway_type.c_str());
			}			
			return requestAttachProcess(session,request,root);			
		}
		case GATEWAY_DESTORY:
			break;		
		case GATEWAY_DETACH:
			break;	
		case GATEWAY_HANGUP:
			break;		
		case GATEWAY_CLAIM:
			break;	
		case GATEWAY_MESSAGE:
			break;	
		case GATEWAY_TRICKLE:
			break;				
		default:
			ret = requestProcessError(request, session_id, transaction, GATEWAY_ERROR_UNKNOWN_REQUEST, "Unknown request '%s'", gateway_type.c_str());
			break;
	}
	return ret;
}

int Gateway::requestAdminProcess(gateway_request request)
{
	RTC_LOG(INFO) << "start " << __FUNCTION__;
}

void Gateway::parseCommonConfig()
{
	bool ipv6;
	if(!gateway_cfg_get_bool(
			config_,gateway_cfg_type_media,"ipv6",&ipv6))
			ipv6 = false;

	/* What is the local IP? */
	RTC_LOG(INFO) << "Selecting local IP address...";
	rtc::BasicNetworkManager network_manager;
	network_manager.StartUpdating();
	// Process pending messages so the network list is updated.
	rtc::Thread::Current()->ProcessMessages(0);
	
#if 1
	std::vector<rtc::Network*> networks;
	network_manager.GetNetworks(&networks);
	networks.erase(std::remove_if(networks.begin(), networks.end(),
																[](rtc::Network* network) {
																	return rtc::kDefaultNetworkIgnoreMask &
																				 network->type();
																}),
								 networks.end());
	if (!networks.empty()) {
		RTC_LOG(LS_WARNING) << "networks.size:" << networks.size();
		for (int i=0;i < networks.size();i++) {
			rtc::Network* network = networks[i];
			RTC_LOG(INFO) << network->ToString();
			const rtc::IPAddress& ip = network->ip();
			RTC_LOG(LS_WARNING) << "ip:" << ip.ToString();
			if (network->active()) {
				local_ip = ip.ToString();
				break;
			}
				
		}
	} else {
		RTC_LOG(LS_WARNING) << "Not enough network adapters for test.";
	}
	network_manager.StopUpdating();
	#endif
	
	if (local_ip.empty())
		local_ip = "127.0.0.1";
	
	RTC_LOG(INFO) << "Using " << local_ip << " as local IP..";

	/* Check if a custom session timeout value was specified */
	if (!gateway_cfg_get_int(config_,
			gateway_cfg_type_general,"session_timeout", &session_timeout)) 
		session_timeout = DEFAULT_SESSION_TIMEOUT;
	
	/* Check if a custom reclaim session timeout value was specified */
	if (!gateway_cfg_get_int(config_,
		gateway_cfg_type_general,"reclaim_session_timeout", &reclaim_session_timeout))
		reclaim_session_timeout = DEFAULT_RECLAIM_SESSION_TIMEOUT;

	bool token_auth = false;
	const char *cert_key = NULL;	
	unsigned char* auth_secret = NULL;
	const char* token_auth_secret = NULL;
	
	if (!gateway_cfg_get_bool(config_,
					gateway_cfg_type_general,"token_auth",&token_auth)) 
		token_auth = false;				
	
	if (!gateway_cfg_get_string(config_,
					gateway_cfg_type_general,"token_auth_secret",&token_auth_secret))
		token_auth_secret = NULL;				
	
	if (!gateway_cfg_get_string(config_,
					gateway_cfg_type_certificates,"cert_key",&cert_key))
		cert_key = NULL;				

	if (token_auth_secret == NULL && cert_key != NULL) {
		unsigned char out[20000];
		int length = jwt_read_key(cert_key,out);
		auth_secret = (unsigned char*)alloca(length + 1);
		memcpy(auth_secret,out,length);
		auth_secret[length] = '\0';
	} else {
		auth_secret = (unsigned char*)token_auth_secret;
	}
	//RTC_LOG(INFO) << "auth_secret " << (char*)auth_secret;
	gateway_auth_init(token_auth,auth_secret);	
	
}

void Gateway::createPlugins()
{
	gateway_plugin plugin = 
			createPlugin(this,GATEWAY_PLUGIN_P2PCALL);
	if (plugin->init(std::string()) == 0) {
		rtc::CritScope lock(&pluginsLock);
		plugins_.insert(
			std::pair<std::string,gateway_plugin>(plugin->getPackage(),plugin)
		);	
	}
}

void Gateway::destoryPlugins()
{
	rtc::CritScope lock(&pluginsLock);
	if (plugins_.empty()) return;
	for(auto iterator=plugins_.begin();
			iterator != plugins_.end(); iterator++) {
		if (iterator->second != nullptr) {
			iterator->second->destory();
			iterator->second = nullptr;
		}
		plugins_.erase(iterator);
	} 
	plugins_.clear();
}

void Gateway::parseNiceConfig()
{
	const char* stun_server;
	if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_nat,"stun_server",&stun_server))
			stun_server = NULL;
	
	int stun_port;
	if (gateway_cfg_get_int(
			config_,gateway_cfg_type_nat,"stun_port",&stun_port)) {
		if (stun_port < 0)
			stun_server = NULL;
	} else {
		stun_port = 3478;
	} 

	
	bool ice_lite;
	if(!gateway_cfg_get_bool(
			config_,gateway_cfg_type_nat,"ice_lite",&ice_lite))
			ice_lite = false;
	
	bool ice_tcp;
	if(!gateway_cfg_get_bool(
			config_,gateway_cfg_type_nat,"ice_tcp",&ice_tcp))
		ice_tcp = false;
	
	bool full_trickle;
	if(!gateway_cfg_get_bool(
			config_,gateway_cfg_type_nat,"full_trickle",&full_trickle))
		full_trickle = false;

	bool ignore_unreachable_ice_server;
	if(!gateway_cfg_get_bool(
			config_,gateway_cfg_type_nat,"ignore_unreachable_ice_server",&ignore_unreachable_ice_server));
		ignore_unreachable_ice_server = false;


	bool ignore_mdns;
	if(!gateway_cfg_get_bool(
			config_,gateway_cfg_type_nat,"ignore_mdns",&ignore_mdns))
			ignore_mdns = false;
	
	const char* nat_1_1_mapping;
	if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_nat,"nat_1_1_mapping",&nat_1_1_mapping))
			nat_1_1_mapping = NULL;
	else {
		//check is valid?
	}
	/* Any TURN server to use in Gateway? */
	const char* turn_server;
	if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_nat,"turn_server",&turn_server))
			stun_server = NULL;
	
	int turn_port;
	if (gateway_cfg_get_int(
			config_,gateway_cfg_type_nat,"turn_port",&turn_port)) {
		if (turn_port < 0)
			turn_server = NULL;
	} else {
		stun_port = 3478;
	}

	const char* turn_type;
	if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_nat,"turn_type",&turn_type))
			turn_type = NULL;

	const char* turn_user;
	if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_nat,"turn_user",&turn_user))
			turn_user = NULL;

	const char* turn_pwd;
	if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_nat,"turn_pwd",&turn_pwd))
			turn_pwd = NULL;
	
	const char* turn_rest_api;
	if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_nat,"turn_rest_api",&turn_rest_api))
			turn_rest_api = NULL;
	
	const char* turn_rest_api_key;
	if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_nat,"turn_rest_api_key",&turn_rest_api_key))
			turn_rest_api_key = NULL;
	
#ifdef HAVE_TURNRESTAPI
	const char* turn_rest_api_method;
	if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_nat,"turn_rest_api_key",&turn_rest_api_method))
			turn_rest_api_method = NULL;
#endif

	bool ipv6;
	if(!gateway_cfg_get_bool(
			config_,gateway_cfg_type_media,"ipv6",&ipv6))
			ipv6 = false;
	
	uint32_t rtp_min_port=0, rtp_max_port=0;
	const char* rtp_port_range = NULL;
	if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_media,"rtp_port_range",&rtp_port_range))
			rtp_port_range = NULL;	
	
	if (rtp_port_range != NULL) {
		std::vector<std::string> list = absl::StrSplit(rtp_port_range, "-");
		if (list.size() == 2) {
			absl::numbers_internal::safe_strtou32_base(list[0], &rtp_min_port, 10);
			absl::numbers_internal::safe_strtou32_base(list[1], &rtp_max_port, 10);
		}
	}
	RTC_LOG(INFO) << "rtp_min_port:" << rtp_min_port << " rtp_max_port:" << rtp_max_port;
#if 0	
	/* Do we need a limited number of static event loops, or is it ok to have one per handle (the default)? */
	item = gateway_config_get(config, config_general, janus_config_type_item, "event_loops");
	if(item && item->value)
		janus_ice_set_static_event_loops(atoi(item->value));
#endif
	/* Initialize the ICE stack now */
	Handle::init(ice_lite, ice_tcp, full_trickle, ignore_mdns, ipv6, rtp_min_port, rtp_max_port);

	if (stun_server != NULL) {
		//network_address public_addr;
		if(Handle::setStunServer(stun_server, stun_port) < 0) {
			if(!ignore_unreachable_ice_server) {
				RTC_LOG(LERROR) << "Invalid STUN address " << stun_server << ":" << stun_port;
				exit(1);
			} else {
				RTC_LOG(LERROR) << "Invalid STUN address " << stun_server << ":" << stun_port << " STUN will be disabled";
			}
		}
		//public_ip = public_addr.ToString();
	}

	if (turn_server != NULL && turn_user != NULL) {
		if (turn_type == NULL)
			turn_type = "udp";
		if(Handle::setTurnServer(
				turn_server, turn_port, turn_type, 
				turn_user, turn_pwd) < 0) {
			if(!ignore_unreachable_ice_server) {
				RTC_LOG(LERROR) << "Invalid TURN address " << turn_server << ":"<< turn_server;
				exit(1);
			} else {
				RTC_LOG(LERROR) << "Invalid STUN address " << turn_server << ":"<< turn_port << " TURN will be disabled";
			}
		}		
	}
	
#ifndef HAVE_TURNRESTAPI
	if(turn_rest_api != NULL || turn_rest_api_key != NULL) {
		RTC_LOG(WARNING) << "A TURN REST API backend specified in the settings, but libcurl support has not been built";
	}
#else
	if(Handle::setTurnRestApi(std::string(turn_rest_api), std::string(turn_rest_api_key), std::string(turn_rest_api_method)) < 0) {
		RTC_LOG(LERROR) 
				<< "Invalid TURN REST API configuration: " 
				<< turn_rest_api << " " << turn_rest_api_key << " " << turn_rest_api_method;
		exit(1);
	}
#endif

	bool nice_debug;
	if(gateway_cfg_get_bool(
			config_,gateway_cfg_type_nat,"nice_debug",&nice_debug) && nice_debug) 
		Handle::debugEnable();/* Enable libnice debugging */
	
	if(stun_server == NULL && turn_server == NULL) {
		/* No STUN and TURN server provided for Gateway: make sure it isn't on a private address */
		bool private_address = false;
		const char *test_ip = nat_1_1_mapping ? nat_1_1_mapping : local_ip.c_str();
		network_address addr;
		if(!rtc::IPFromString(test_ip, &addr)) {
			RTC_LOG(WARNING) <<  "Invalid address " << test_ip;
		} else {
			private_address = rtc::IPIsPrivate(addr);
		}
		
		if(private_address) {
			RTC_LOG(WARNING) << "Gateway is deployed on a private address " << test_ip
												<< " but you didn't specify any STUN server! Expect trouble if this is supposed to work over the internet and not just in a LAN...";
		}
	}
	RTC_LOG(WARNING) << "parseNiceConfig End.... ";
}

/*! \brief Signal handler (just used to intercept CTRL+C and SIGTERM) */
static void gateway_handle_signal(int signum) 
{
	stop_signal = signum;
	switch(atomic_int_get(&stop)) {
		case 0:
			printf("Stopping server, please wait...\n");
			break;
		case 1:
			printf("In a hurry? I'm trying to free resources cleanly, here!\n");
			break;
		default:
			printf("Ok, leaving immediately...\n");
			break;
	}

	atomic_int_inc(&stop);
	if(atomic_int_get(&stop) > 2)
		exit(1);

	if (gateway_) {
		gateway_.reset(nullptr);
		gateway_ = nullptr;
	}
	
	if(main_thread != nullptr) {
		main_thread->Stop();	
		main_thread.reset(nullptr);
		main_thread = nullptr;
	}
}

/*! \brief Termination handler (atexit) */
static void gateway_termination_handler(void) 
{
	/* If we're daemonizing, we send an error code to the parent */
	if(daemonize) {
		int code = 1;
		ssize_t res = 0;
		do {
			res = write(pipefd[1], &code, sizeof(int));
		} while(res == -1 && errno == EINTR);
	}
}

int Gateway::StartArgs(args_info_t& args_info)
{
	/* Core dumps may be disallowed by parent of this process; change that */
	struct rlimit core_limits;
	core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &core_limits);

	/* Any configuration to open? */
	if(args_info.config_given) {
		config_file = strdup(args_info.config_arg);
	}
	
	if(args_info.configs_folder_given) {
		configs_folder = strdup(args_info.configs_folder_arg);
	} else {
		//configs_folder = strdup(CONFDIR);
		configs_folder = strdup(DEFAULT_CONFDIR);
	}

	if(config_file == NULL) {
		char file[255];
		snprintf(file, 255, "%s/gateway.cfg", configs_folder);
		config_file = strdup(file);
	}
	
	if((config_ = gateway_cfg_parse(config_file)) == NULL) {
		/* We failed to load the libconfig configuration file, let's try the INI */
		printf("Failed to load %s, trying the INI instead...\n", config_file);
		free(configs_folder);
		free(config_file);
		exit(1);
	}	

	/* Check if we need to log to console and/or file */
	bool log_to_stdout = true;
	if(args_info.disable_stdout_given) {
		log_to_stdout = false;
		gateway_cfg_set_bool(config_,
			gateway_cfg_type_general,"log_to_stdout", 0);
	} else {
		/* Check if the configuration file is saying anything about this */
		if(!gateway_cfg_get_bool(
			config_,gateway_cfg_type_general,"log_to_stdout",&log_to_stdout))
			log_to_stdout = false;
	}	
	
	const char *logfile = NULL;
	if(args_info.log_file_given) {
		logfile = args_info.log_file_arg;
		if (logfile != NULL)
			gateway_cfg_set_string(config_, 
				gateway_cfg_type_general, "log_to_file", logfile);
	} else {
		/* Check if the configuration file is saying anything about this */
		if(!gateway_cfg_get_string(
			config_,gateway_cfg_type_general,"log_to_file",&logfile))
			logfile = NULL;
	}
	
	/* Check if we're going to daemonize Gateway */
	if(args_info.daemon_given) {
		daemonize = true;
		gateway_cfg_set_bool(config_,
			gateway_cfg_type_general,"daemonize", 1);		
	} else {
		/* Check if the configuration file is saying anything about this */
		if(!gateway_cfg_get_bool(
			config_,gateway_cfg_type_general,"daemonize",&daemonize))
			daemonize = false;
	}
	
	/* If we're going to daemonize, make sure logging to stdout is disabled and a log file has been specified */
	if(daemonize && log_to_stdout) {
		log_to_stdout = false;
		gateway_cfg_set_bool(config_,
			gateway_cfg_type_general,"log_to_stdout", 0);
	}
	
	if(daemonize && logfile == NULL) {
		printf("Running Gateway as a daemon but no log file provided, giving up...\n");
		exit(1);
	}

	/* Daemonize now, if we need to */
	if(daemonize) {
		printf("Running Gateway as a daemon\n");
		/* Create a pipe for parent<->child communication during the startup phase */
		if(pipe(pipefd) == -1) {
			printf("pipe error!\n");
			exit(1);
		}

		/* Fork off the parent process */
		pid_t pid = fork();
		if(pid < 0) {
			printf("Fork error!\n");
			exit(1);
		}
		
		if(pid > 0) {
			/* Ok, we're the parent: let's wait for the child to tell us everything started fine */
			close(pipefd[1]);
			int code = -1;
			struct pollfd pollfds;

			while(code < 0) {
				pollfds.fd = pipefd[0];
				pollfds.events = POLLIN;
				int res = poll(&pollfds, 1, -1);
				if(res < 0)
					break;
				if(res == 0)
					continue;
				if(pollfds.revents & POLLERR || pollfds.revents & POLLHUP)
					break;
				if(pollfds.revents & POLLIN) {
					res = read(pipefd[0], &code, sizeof(int));
					break;
				}
			}
			if(code < 0)
				code = 1;

			/* Leave the parent and return the exit code we received from the child */
			if(code)
				printf("Error launching Gateway (error code %d), check the logs for more details\n", code);
			exit(code);
		}
		
		/* Child here */
		close(pipefd[0]);

		/* Change the file mode mask */
		umask(0);

		/* Create a new SID for the child process */
		pid_t sid = setsid();
		if(sid < 0) {
			printf("Error setting SID!\n");
			exit(1);
		}
		
		#if 0
		/* Change the current working directory */
		const char *cwd = 
			(args_info.cwd_path_given) ? args_info.cwd_path_arg : "./";
		if((chdir(cwd)) < 0) {
			printf("Error changing the current working directory!\n");
			exit(1);
		}
		#endif
		/* We close stdin/stdout/stderr when initializing the logger */
	}	
	
	/* Was a custom instance name provided? */
	if(args_info.server_name_given) {
		gateway_cfg_set_string(config_,
			gateway_cfg_type_general,"server_name", args_info.server_name_arg);				
	}
	
	if(!gateway_cfg_get_string(
		config_,gateway_cfg_type_general,"server_name",&server_name))
		server_name = "gateway";
	
	int debug_level = 5;
	if(args_info.debug_level_given) {
		debug_level = args_info.debug_level_arg;
		gateway_cfg_set_int(config_,
			gateway_cfg_type_general,"debug_level", args_info.debug_level_arg);
	} else {
		/* No command line directive on logging, try the configuration file */
		if(!gateway_cfg_get_int(
			config_,gateway_cfg_type_general,"debug_level",&debug_level))
			debug_level = 5;
	}

	switch(debug_level) {
		case 0:
		case 1:
			debug_level = rtc::LERROR;
			break;
		case 2:
		case 3:
			debug_level = rtc::WARNING;
			break;			
		case 4:
		case 5:
			debug_level = rtc::INFO;
			break;				
		case 6:
		case 7:
			debug_level = rtc::LS_VERBOSE;
			break;			
		default:
			break;
	}
	
	if(args_info.debug_timestamps_given) {
		gateway_cfg_set_bool(config_,
			gateway_cfg_type_general,"debug_timestamps", true);		
	}

	bool debug_timestamps = false;
	if (!gateway_cfg_get_bool(config_,
			gateway_cfg_type_general,"debug_timestamps", &debug_timestamps))
			debug_timestamps = false;
	
	if(args_info.session_timeout_given)
		gateway_cfg_set_int(config_,
			gateway_cfg_type_general,"session_timeout", 
			args_info.session_timeout_arg);	
	
	if(args_info.reclaim_session_timeout_given) 
		gateway_cfg_set_int(config_,
			gateway_cfg_type_general,"reclaim_session_timeout", 
				args_info.reclaim_session_timeout_arg);			

 	if(args_info.interface_given)
		gateway_cfg_set_string(config_,
			gateway_cfg_type_general,"interface", 
				args_info.interface_arg);				
	
	if(args_info.configs_folder_given) 
		gateway_cfg_set_string(config_,
			gateway_cfg_type_general,"configs_folder", 
				args_info.configs_folder_arg);			
	
	if(args_info.plugins_folder_given) 
		gateway_cfg_set_string(config_,
			gateway_cfg_type_general,"plugins_folder", 
				args_info.plugins_folder_arg);			
	
	if(args_info.apisecret_given) 
		gateway_cfg_set_string(config_,
			gateway_cfg_type_general,"api_secret", 
				args_info.apisecret_arg);		
	
	if(args_info.token_auth_given) 
		gateway_cfg_set_bool(config_,
			gateway_cfg_type_general,"token_auth",true);				
	
	if(args_info.token_auth_secret_given) 
		gateway_cfg_set_string(config_,
			gateway_cfg_type_general,"token_auth_secret",args_info.token_auth_secret_arg);				

	if(args_info.no_webrtc_encryption_given) 
		gateway_cfg_set_bool(config_,
			gateway_cfg_type_general,"no_webrtc_encryption",true);	


	if(args_info.cert_pem_given) 
		gateway_cfg_set_string(config_,
			gateway_cfg_type_general,"cert_pem", 
				args_info.cert_pem_arg);	
	
	if(args_info.cert_key_given) 
		gateway_cfg_set_string(config_,
			gateway_cfg_type_general,"cert_key", 
				args_info.cert_key_arg);		
	
	if(args_info.cert_pwd_given) 
		gateway_cfg_set_string(config_,
			gateway_cfg_type_general,"cert_pwd", 
				args_info.cert_pwd_arg);		
	
	if(args_info.stun_server_given) {
		/* Split in server and port (if port missing, use 3478 as default) */
		char *stunport = strrchr(args_info.stun_server_arg, ':');
		if(stunport != NULL) {
			*stunport = '\0';
			stunport++;
			gateway_cfg_set_string(config_, 
				gateway_cfg_type_nat, "stun_server", args_info.stun_server_arg);
			uint32_t port = 0;
			absl::numbers_internal::safe_strtou32_base(stunport, &port, 10);
			gateway_cfg_set_int(config_, 
				gateway_cfg_type_nat, "stun_port", port);
		} else {
			gateway_cfg_set_string(config_, 
				gateway_cfg_type_nat, "stun_server", args_info.stun_server_arg);
			gateway_cfg_set_int(config_, 
				gateway_cfg_type_nat, "stun_port", 3478);
		}
	}

	if(args_info.nat_1_1_given) 
		gateway_cfg_set_string(config_, 
			gateway_cfg_type_nat, "nat_1_1_mapping", args_info.nat_1_1_arg);
	
	if(args_info.ice_enforce_list_given) 
		gateway_cfg_set_string(config_, 
			gateway_cfg_type_nat, "ice_enforce_list", args_info.ice_enforce_list_arg);		
	
	if(args_info.ice_ignore_list_given) 
		gateway_cfg_set_string(config_, 
			gateway_cfg_type_nat, "ice_ignore_list", args_info.ice_ignore_list_arg);
	
	if(args_info.libnice_debug_given) 
		gateway_cfg_set_bool(config_, 
			gateway_cfg_type_nat, "nice_debug", true);			
	
	if(args_info.full_trickle_given) 
		gateway_cfg_set_bool(config_, 
			gateway_cfg_type_nat, "full_trickle", true);			
	
	if(args_info.ice_lite_given) 
		gateway_cfg_set_bool(config_, 
			gateway_cfg_type_nat, "ice_lite", true);			
	
	if(args_info.ice_tcp_given) 
		gateway_cfg_set_bool(config_, 
			gateway_cfg_type_nat, "ice_tcp", true);			
	
	if(args_info.ipv6_candidates_given) 
		gateway_cfg_set_bool(config_, 
			gateway_cfg_type_media, "ipv6", true);		

	if(args_info.min_nack_queue_given) 
		gateway_cfg_set_int(config_, 
			gateway_cfg_type_media, "min_nack_queue", args_info.min_nack_queue_arg);	
	
	if(args_info.no_media_timer_given) {
		gateway_cfg_set_int(config_, 
			gateway_cfg_type_media, "no_media_timer", args_info.no_media_timer_arg);	
	}
	
	if(args_info.slowlink_threshold_given)
		gateway_cfg_set_int(config_, 
			gateway_cfg_type_media, "slowlink_threshold", args_info.slowlink_threshold_arg);			
	
	if(args_info.twcc_period_given)
		gateway_cfg_set_int(config_, 
			gateway_cfg_type_media, "twcc_period", args_info.twcc_period_arg);			
	
	if(args_info.rtp_port_range_given) 
		gateway_cfg_set_string(config_, 
			gateway_cfg_type_media, "rtp_port_range", args_info.rtp_port_range_arg);					
	
	if(args_info.event_handlers_given) {
		gateway_cfg_set_bool(config_, 
			gateway_cfg_type_events, "broadcast", true);			
	}

	if (debug_timestamps)
		rtc::LogMessage::LogTimestamps(true);

	rtc::LogMessage::LogToDebug(rtc::LoggingSeverity(debug_level));
	if(daemonize && logfile) {
		rtc::LogMessage::SetLogToStderr(false);
		LogFileSink* sink = 
			new LogFileSink(logfile);
    rtc::LogMessage::AddLogToStream(sink, rtc::LoggingSeverity(debug_level)); 		
	} else {
		rtc::LogMessage::SetLogToStderr(true);
	}
	/* Handle SIGINT (CTRL-C), SIGTERM (from service managers) */
	signal(SIGINT, gateway_handle_signal);
	signal(SIGTERM, gateway_handle_signal);
	atexit(gateway_termination_handler);
	
	printf("---------------------------------------------------\n");
	printf("  Starting Ubonass Gateway (WebRTC Server) v1.0.0\n");
	printf("---------------------------------------------------\n\n");
	gateway_ = absl::make_unique<Gateway>();
	
	main_thread = rtc::Thread::Create();
	
	main_thread->Run();

	if (gateway_)
		gateway_.reset(nullptr);
	
	if (main_thread)
		main_thread.reset(nullptr);
	
	if (config_ != NULL)
		gateway_cfg_destory(config_);	
	config_ = NULL;

	if (config_file != NULL)
		free(config_file); 
	config_file = NULL;

	if (configs_folder != NULL)
		free(configs_folder); 
	configs_folder = NULL;

	task_queue_factory_.reset(nullptr);

	printf("------------------------gateway exit-----------------------\n\n");
	return 0;
}
