

#include "p2p_call.h"
#include "handle/handle.h"
#include "atomic.h"
#include "rtc_base/logging.h"
#include "rtc_base/null_socket_server.h"

/* Plugin information */
#define GATEWAY_P2PCALL_VERSION				6
#define GATEWAY_P2PCALL_VERSION_STRING	"0.0.6"
#define GATEWAY_P2PCALL_DESCRIPTION		"This is a simple p2p call plugin for Gateway"
#define GATEWAY_P2PCALL_NAME						"Gateway P2pCall plugin"
#define GATEWAY_P2PCALL_AUTHOR					"Jeffrey"
#define GATEWAY_P2PCALL_PACKAGE				"gateway.plugin.p2pcall"

/* Error codes */
#define GATEWAY_VIDEOCALL_ERROR_UNKNOWN_ERROR			499
#define GATEWAY_VIDEOCALL_ERROR_NO_MESSAGE			470
#define GATEWAY_VIDEOCALL_ERROR_INVALID_JSON			471
#define GATEWAY_VIDEOCALL_ERROR_INVALID_REQUEST		472
#define GATEWAY_VIDEOCALL_ERROR_REGISTER_FIRST		473
#define GATEWAY_VIDEOCALL_ERROR_INVALID_ELEMENT		474
#define GATEWAY_VIDEOCALL_ERROR_MISSING_ELEMENT		475
#define GATEWAY_VIDEOCALL_ERROR_USERNAME_TAKEN		476
#define GATEWAY_VIDEOCALL_ERROR_ALREADY_REGISTERED	477
#define GATEWAY_VIDEOCALL_ERROR_NO_SUCH_USERNAME		478
#define GATEWAY_VIDEOCALL_ERROR_USE_ECHO_TEST			479
#define GATEWAY_VIDEOCALL_ERROR_ALREADY_IN_CALL		480
#define GATEWAY_VIDEOCALL_ERROR_NO_CALL				481
#define GATEWAY_VIDEOCALL_ERROR_MISSING_SDP			482
#define GATEWAY_VIDEOCALL_ERROR_INVALID_SDP			483

//static gateway_videocall_message exit_message;

class P2pCall::P2pCallSession : public PluginSession {
 public:
 	P2pCallSession();
	virtual ~P2pCallSession();	
 	std::string username;
	volatile int incall;
	volatile int hangingup;
	volatile int destroyed;	
 	gateway_plugin_session peer;
};

P2pCall::P2pCallSession::P2pCallSession()
{
	atomic_int_set(&incall, 0);
	atomic_int_set(&hangingup, 0);
	atomic_int_set(&destroyed, 0);
}

P2pCall::P2pCallSession::~P2pCallSession()
{
	RTC_LOG(INFO) << "enter :" << __FUNCTION__;
}


P2pCall::P2pCall(PluginCallbacks* callback)
	: rtc::Thread(std::unique_ptr<rtc::SocketServer>(new rtc::NullSocketServer())),
	  config_(NULL), 
	  callback_(callback)
{
	RTC_DCHECK(callback != NULL);
	//exit_message.exit = true;
	//exit_message.handle = nullptr;
	atomic_int_set(&initialized,0);
	atomic_int_set(&stopping,0);
}

P2pCall::~P2pCall()
{
	
	RTC_LOG(INFO) << __FUNCTION__ << "@@@@@@@@@@@@";	
	
	if(atomic_int_get(&initialized)) 
		destory();
	callback_ = NULL;
}
/*
 * @init p2p call resource
 */ 
int P2pCall::init(const std::string& config_path)
{	

	if(atomic_int_get(&stopping)) {
		/* Still stopping from before */
		return -1;
	}
	
#if 0
	if(callback_ == NULL || config_path.empty()) {
		/* Invalid arguments */
		return -1;
	}
	config_path_ = config_path;
	/* Read configuration */
	std::string filename = GATEWAY_P2PCALL_PACKAGE;
	filename.append(".cfg");
	RTC_LOG(INFO) << "Configuration file: "
								<< config_path_
								<< filename;
	config_ = gateway_cfg_parse(config_path_.c_str(),filename.c_str());
	if(config_ == NULL) {
		RTC_LOG(WARNING) << "Couldn't find .cfg configuration file "
									<< GATEWAY_P2PCALL_PACKAGE;
	}
	/*parase something...*/

	if (config_ != NULL)
		gateway_cfg_destory(config_);
	config_ = NULL;	
#endif
	//async_queue_init(&messages);
	memset(&messages,sizeof(async_queue),0);
	async_queue_init(&messages,true,16,NULL,NULL);
	atomic_int_set(&initialized, 1);
	/* Launch the thread that will handle incoming messages */
	this->SetName("p2p_call_thread", nullptr);
	if (!this->Start()) {
		atomic_int_set(&initialized, 0);
		RTC_LOG(LERROR) << "start p2p call thread error";
		return -1;
	}
	RTC_LOG(INFO) << GATEWAY_P2PCALL_NAME << " initialized";
	return 0;	
}

/*
 * @release plugin source and diable it
 */ 	
void P2pCall::destory()
{
	RTC_LOG(INFO) << __FUNCTION__;	

	if(!atomic_int_get(&initialized))
		return;

	if (config_ != NULL)
		gateway_cfg_destory(config_);
	config_ = NULL;

	atomic_int_set(&stopping, 1);

#if 0	
	async_queue_element* element = 
			async_queue_element_new(&messages.pool);
	element->data = NULL;
	async_queue_push(&messages, element);
#endif	
	this->Join();
	{
		rtc::CritScope lock(&sessionsLock);
		if (!sessions.empty()) {
			for(auto iterator = sessions.begin(); 
					iterator != sessions.end(); iterator++) {
				if (iterator->second != nullptr) 
					iterator->second = nullptr;				
				sessions.erase(iterator);
			} 
		}
		if (!usernames.empty()) 
			usernames.clear();		
	}
	async_queue_clean(&messages);
	atomic_int_set(&initialized, 0);
	atomic_int_set(&stopping, 0);
	RTC_LOG(INFO) << GATEWAY_P2PCALL_NAME  << " destroyed!";
}

/*
 *@params out error
 * return plugin session
 */
PluginType P2pCall::getType()
{
	return GATEWAY_PLUGIN_P2PCALL;
}
/*
 *@params out error
 * return plugin session
 */
std::string P2pCall::getPackage()
{
	return GATEWAY_P2PCALL_PACKAGE;
}

std::string P2pCall::getName()
{
	return "p2pcall";
}

/*
*@params out error
* return plugin session
*/
gateway_plugin_session P2pCall::createSession(const gateway_handle& handle,int *error)
{
	if(atomic_int_get(&stopping) || !atomic_int_get(&initialized)) {
		*error = -1;
		return nullptr;
	}
	gateway_plugin_session session = 
					new rtc::RefCountedObject<P2pCallSession>(); 
	session->g_handle = handle;
	{
		rtc::CritScope lock(&sessionsLock);
		sessions.insert(
			std::pair<gateway_plugin_session,gateway_plugin_session>
			(session,session)
		);		
	}
	atomic_int_set(&session->stopped, 0);	
	return session;
}

/**
* destory the session
*/
void P2pCall::destorySession(const gateway_plugin_session& session, int *error)
{
	if(atomic_int_get(&stopping) || !atomic_int_get(&initialized)) {
		*error = -1;
		return;
	}
	
	if (session == nullptr) {
		*error = -2;
		return;	
	}
	
	{
		rtc::CritScope lock(&sessionsLock);
		if (sessions.empty()) {
			RTC_LOG(LERROR) << "No session associated with this handle...";
			*error = -2;
			return;
		}
		
		auto iterator = sessions.find(session);
		if (iterator == sessions.end()) {
			RTC_LOG(LERROR) << "No session associated with this handle...";
			*error = -2;
			return;
		}
		
		if (iterator->second != nullptr) {
			P2pCallSession* call_session = 
					static_cast<P2pCallSession*>(iterator->second.get());
			if (call_session != NULL && !call_session->username.empty()) {
				//janus_videocall_hangup_media_internal(handle);
				auto iter = usernames.find(call_session->username);
				if (iter != usernames.end())
					usernames.erase(iter);
			}
			//iterator->second = nullptr;
		}
		sessions.erase(iterator);
	}
	session->g_handle = nullptr;
}

gateway_plugin_result P2pCall::handleMessage(
	const gateway_plugin_session& session, 
	const std::string& transaction, 
	const json_value& message, 
	const json_value& jsep) 
{
	RTC_LOG(INFO) << "enter :" << __FUNCTION__;
	if(atomic_int_get(&stopping) || !atomic_int_get(&initialized))
		return gateway_plugin_result_new(GATEWAY_PLUGIN_ERROR, atomic_int_get(&stopping) ? "Shutting down" : "Plugin not initialized", std::string());

	if(session == nullptr)
		return gateway_plugin_result_new(GATEWAY_PLUGIN_ERROR, "No session associated with this handle", std::string());

	P2pCallSession* call_session = NULL;
	{
		rtc::CritScope lock(&sessionsLock);
		call_session = lookupP2pSession(session);		
	}
	
	if(!call_session) 
		return gateway_plugin_result_new(GATEWAY_PLUGIN_ERROR, "No session associated with this handle", std::string());

	async_queue_element *element = 
			async_queue_element_new(messages.pool);
	element->data = 
			(void *)gateway_videocall_message_new(session,transaction,message,jsep); 
	async_queue_push(&messages, element);
	/* All the requests to this plugin are handled asynchronously */
	return gateway_plugin_result_new(GATEWAY_PLUGIN_OK_WAIT, std::string(), std::string());	
}

/**
*return the json string session info
*/
std::string P2pCall::querySession(const gateway_plugin_session& session)
{
	return std::string();
}

P2pCall::P2pCallSession* P2pCall::lookupP2pSession(const gateway_plugin_session& session) 
{
	P2pCallSession *session_ = NULL;
	if (sessions.count(session)) {
		session_ = static_cast<P2pCallSession*>(sessions[session].get());
	}
	return session_;
}

int P2pCall::processError(
	const gateway_plugin_session& session,
	int error_code,
	const char* error_cause,
	const std::string& transaction)
{
	/* Prepare JSON error event */
	json_value event;
	event["videocall"] = "event";
	event["error_code"] = error_code;
	event["error"] = error_cause;
	int ret = 0;
	if (callback_)
		ret = callback_->sendEvent(this, session, transaction, std::move(event), std::string());
	return ret;
}

void P2pCall::Run() 
{
	RTC_LOG(INFO) << "Joining VideoCall handler thread" ;
	gateway_videocall_message *msg = NULL;
	int error_code = 0;
	char error_cause[512];
	while(atomic_int_get(&initialized) && !atomic_int_get(&stopping)) {
		async_queue_element* element = async_queue_pop_wait(&messages,-1);
		if (element == NULL)
			continue;
		if (element->data == NULL) {
			async_queue_element_free(messages.pool,element);
			break;
		}
		msg = (gateway_videocall_message *)element->data;
		if(msg->session == nullptr) {
			gateway_videocall_message_free(msg);
			async_queue_element_free(messages.pool,element);
			continue;
		}	
		
		P2pCallSession* session = NULL;
		{
			rtc::CritScope lock(&sessionsLock);
			session = lookupP2pSession(msg->session); 	
			if(session == NULL) {
				RTC_LOG(LERROR) << "No session associated with this handle...";
				gateway_videocall_message_free(msg);
				async_queue_element_free(messages.pool,element);
				continue;
			}
			if(atomic_int_get(&session->destroyed)) {
				gateway_videocall_message_free(msg);
				async_queue_element_free(messages.pool,element);
				continue;
			}			
		}	
		
		/* Handle request */
		const json_value& root = msg->message;
		if(root.empty()) {
			RTC_LOG(LERROR) << "No message??";
			error_code = GATEWAY_VIDEOCALL_ERROR_NO_MESSAGE;
			snprintf(error_cause, 512, "%s", "No message??");
			processError(msg->session,error_code,error_cause,msg->transaction);
			gateway_videocall_message_free(msg);
			async_queue_element_free(messages.pool,element);			
			continue;
		}
	}
}

void gateway_videocall_message_free(gateway_videocall_message *msg) 
{
	if(!msg)
		return;
	
	if(msg->session) 
		msg->session = nullptr;
	
	if (!msg->transaction.empty())
		msg->transaction.clear();
	
	if (!msg->message.empty())
		msg->message.clear();	

	if (!msg->jsep.empty())
		msg->jsep.clear();	

	free(msg);
	
	msg = NULL;
}

gateway_videocall_message* gateway_videocall_message_new(
	const gateway_plugin_session& session, 
	const std::string& transaction, 
	const json_value& message, 
	const json_value& jsep) 
{
	gateway_videocall_message *msg = 
				(gateway_videocall_message*)malloc(sizeof(gateway_videocall_message));
	msg->session = session;
	msg->transaction = transaction;
	msg->message = std::move(message);
	msg->jsep = std::move(jsep);
	return msg;
}
