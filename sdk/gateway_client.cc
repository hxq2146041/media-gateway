#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "absl/algorithm/container.h"
#include "system_wrappers/include/clock.h"
#include "rtc_base/logging.h"
#include "rtc_base/thread.h"
#include "rtc_base/strings/json.h"
#include "rtc_base/checks.h"
#include "rtc_base/synchronization/sequence_checker.h"

#include "utils.h"
#include "helper.h"
#include "atomic.h"
#include "plugin_handle.h"

#include "gateway_client.h"

static const char* s_token = 
		"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhZG1pbiI6ImZhbHNlIiwiYXVkIjoiY3VzdG1lciIsImV4cCI6MTkwNDQ3MDQ2MDAwMCwiaXNzIjoiZ2F0ZXdheSIsInBsdWdpbiI6InAycGNhbGwudmlkZW9jYWxsIiwic3ViIjoiZGV2ZWxvcGVyIn0._uMn5FH41iIOBU4ied6Wlv1IK3Hb_Ays7G0PIHJG4Ro";
#define GATEWAY_WEBSOCKETS_PORT	8989
#define GATEWAY_WEBSOCKETS_URL	"192.168.2.229"

/**
 *when using client sdk
 */ 
GatewayClient::GatewayClient(IGatewayCallback* callback)
	: clock_(webrtc::Clock::GetRealTimeClock()),
		callback_(callback),
		session_(NULL),
		token(s_token)
{
	atomic_int_set(&session_activity,0);
	gateway_thread_ = rtc::Thread::Create();
	gateway_thread_->SetName("GatewayThread", nullptr);
	RTC_CHECK(gateway_thread_->Start()) << "Failed to start thread";	
	RTC_LOG(INFO) << "@@@@";
	transport_ = createTransport(TRANSPORT_WEBSOCKET,this);
	transport_->connect(GATEWAY_WEBSOCKETS_URL,GATEWAY_WEBSOCKETS_PORT);
}
/**
 *
 */
GatewayClient::~GatewayClient()
{

}
/**
 *send message when client using
 */
void GatewayClient::sendMessage(const std::string message)
{

}

/**
 * server request
 */
void GatewayClient::IncomingRequest(
	gateway_transport* transport,
	gateway_transport_session& session,
	void *request_id, bool admin,char* message)
{
  if (!gateway_thread_->IsCurrent()) {
  	gateway_thread_->Invoke<void>(RTC_FROM_HERE, [&] {
				RTC_DCHECK_RUN_ON(gateway_thread_.get());
				this->IncomingRequest(transport,session,request_id,admin,message);
    });
		return;
  }
	
	if (transport == NULL || session == nullptr || message == NULL)
		return ;
	
	RTC_LOG(INFO) << "Start :" << __FUNCTION__ << " " << message;
  Json::Reader reader;
  Json::Value obj;
  if (!reader.parse(message, obj)) {
		std::string error = "Received unknown message. ";
		error.append(message);
    RTC_LOG(WARNING) << error;
    return ;
  } 

	#if 1
	if (message) {
		free(message);
		message = NULL;
	}
	#endif
	
	uint64_t sender = 0;
	std::string gateway;
	std::string transaction;	
	
	if (!rtc::GetStringFromJsonObject(obj, "gateway",&gateway)) {
		RTC_LOG(WARNING) << "Can't parse received message.";		
		return;
	}
	
	if (obj.isMember("transaction")) {
		if (!rtc::GetStringFromJsonObject(obj, "transaction",&transaction)) {
			RTC_LOG(WARNING) << "Can't parse received transaction.";	
			return;
		}
	}	

	if (obj.isMember("sender")) {
		if (!rtc::GetUInt64FromJsonObject(obj, "sender",&sender)) {
			RTC_LOG(WARNING) << "Can't parse received message.";		
			return;
		}
	}

	switch (gatewayTypeFromString(gateway.c_str())) {
			case GATEWAY_KEEPALIVE:
					break;
			case GATEWAY_ACK:
			case GATEWAY_SUCCESS: 
			case GATEWAY_ERROR: 
				if (!transaction.empty()) {
					rtc::CritScope lock(&transactionsLock);
					std::unique_ptr<ITransaction> cb;
					if (transactions.count(transaction))
						cb.reset(transactions[transaction].release());
					if (cb) {
						cb->reportSuccess(obj);
						transactions.erase(transaction);
						cb.reset(nullptr);
					}
				} 
			break;

			case GATEWAY_HANGUP: 
			break;
			
			case GATEWAY_DETACHED: 
			break;
			
			case GATEWAY_EVENT: 
			break;
			
			case GATEWAY_TRICKLE: 
			break;

			case GATEWAY_MEDIA: 
				break;

			case GATEWAY_WEBRTCUP: 
			break;
			
			case GATEWAY_SLOWLINK: 
			break;
			default:		
				break;
	}
}

/**
 * gone
 */
void GatewayClient::TransportGone(
	gateway_transport* transport,
	gateway_transport_session& session)
{
	RTC_LOG(INFO) << "Start :" << __FUNCTION__;

}
/**
 * @param session: gateway session
 * @param event : jsson event message
 */
void GatewayClient::NotifyEvent(
	gateway_transport* transport,
	gateway_transport_session& session,
	const std::string& message)
{
  if (!gateway_thread_->IsCurrent()) {
  	gateway_thread_->Invoke<void>(RTC_FROM_HERE, [&] {
				RTC_DCHECK_RUN_ON(gateway_thread_.get());
				this->NotifyEvent(transport,session,message);
    });
		return;
  }
	
  Json::Reader reader;
  Json::Value obj;
	std::string event;
  if (!reader.parse(std::move(message), obj)) {
		std::string error = "Received unknown message. " + message;
    RTC_LOG(WARNING) << error;
    return;
  }

	RTC_LOG(WARNING) << "obj:" << rtc::JsonValueToString(obj);
	
	if (!rtc::GetStringFromJsonObject(obj, "event",&event)) {
		RTC_LOG(WARNING) << "Can't parse received event message.";		
		return;
	}
	
	if (!strcmp(event.c_str(), "connected") && session_ == nullptr) {
		session_ = session;
		createSession();
		//gatewayInfo();
	}
}

void GatewayClient::gatewayInfo()
{
  if (!gateway_thread_->IsCurrent()) {
  	gateway_thread_->Invoke<void>(RTC_FROM_HERE, [&] {
				RTC_DCHECK_RUN_ON(gateway_thread_.get());
				this->gatewayInfo();
    });
		return;
  }
	
	RTC_DCHECK(transport_);
	RTC_DCHECK(session_);
  Json::Value obj;	
  obj["gateway"] = "info";
	if (!token.empty())
		obj["token"] = token;
	Json::StyledWriter writer;
	transport_->sendMessage(session_,writer.write(obj));
}
/**
 *@ param transaction
 */
const std::string GatewayClient::putNewTransaction(
		std::unique_ptr<ITransaction> transaction)
{
	RTC_CHECK(transaction);
	std::string id = gateway_random_id_string();
	rtc::CritScope lock(&transactionsLock);
	while (transactions.find(id) != transactions.end())
		id = gateway_random_id_string();
	transactions.insert(
		std::pair<std::string,std::unique_ptr<ITransaction>>(
			id,std::move(transaction)));
	return id;
}

#if 0
void GatewayClient::attach(PluginType type)
{
	if (!atomic_int_get(&session_activity))
		event.Wait(-1);
	
	Json::Value obj;
	obj["gateway"] = "attach";
	obj["plugin"] = pluginTypeToString(type);
		
	if (transport_->getType() == TRANSPORT_WEBSOCKET)
		obj["session_id"] = session_id;	
	
	std::unique_ptr<ITransaction> transaction = 
			TransactionFactory::createNewTransaction(TRANSACTION_ATTACH,GATEWAY_PLUGIN_P2PCALL,this,NULL);
	std::string id  = putNewTransaction(std::move(transaction));
	obj["transaction"] = id;
	if (!token.empty())
		obj["token"] = token;

	if (gateway_thread_  && transport_) {
		gateway_thread_->Invoke<void>(RTC_FROM_HERE, [&] {
				RTC_DCHECK_RUN_ON(gateway_thread_.get()); 	
				Json::StyledWriter writer;
				transport_->sendMessage(session_,writer.write(obj));				
		}); 
	} 
}
#endif

void GatewayClient::callP2p(
	const std::string& peerName,const std::string& sdp)
{
	if (!atomic_int_get(&session_activity))
		event.Wait(-1);
	
	std::vector<std::string> data_;
	data_.push_back(peerName);
	Json::Value array = rtc::StringVectorToJsonArray(data_);
	
	Json::Value ask;
	ask["type"] = "call";
	ask["data"] = array;

	Json::Value obj;
	obj["gateway"] = "ask";
	obj["plugin"] = pluginTypeToString(GATEWAY_PLUGIN_P2PCALL);
	obj["ask"] = ask;
	
	if (transport_->getType() == TRANSPORT_WEBSOCKET)
		obj["session_id"] = session_id;	

	askInfo* info = new askInfo(); 	
	info->peerName = peerName;
	info->sdp = sdp;
	info->type = "offer";
	std::unique_ptr<ITransaction> transaction = 
			TransactionFactory::createAskTransaction(
				GATEWAY_PLUGIN_P2PCALL,
				std::move(info),this);	
	std::string id  = putNewTransaction(std::move(transaction));
	obj["transaction"] = id;
	if (!token.empty())
		obj["token"] = token;
	if (gateway_thread_  && transport_) {
		gateway_thread_->Invoke<void>(RTC_FROM_HERE, [&] {
				RTC_DCHECK_RUN_ON(gateway_thread_.get()); 	
				Json::StyledWriter writer;
				transport_->sendMessage(session_,writer.write(obj));				
		}); 
	} 
}
/**
 *@ param null
 */ 
void GatewayClient::createSession()
{
  if (!gateway_thread_->IsCurrent()) {
  	gateway_thread_->Invoke<void>(RTC_FROM_HERE, [&] {
				RTC_DCHECK_RUN_ON(gateway_thread_.get());
				this->createSession();
    });
		return;
  }
	
	RTC_DCHECK(session_);
  Json::Value obj;	
	
  obj["gateway"] = "create";
	std::unique_ptr<ITransaction> transaction = 
			TransactionFactory::createNewTransaction(TRANSACTION_CREATE,this);
	std::string id  = putNewTransaction(std::move(transaction));
	obj["session_uid"] = "12345";
	obj["transaction"] = id;
	if (!token.empty())
		obj["token"] = token;
	Json::StyledWriter writer;
	transport_->sendMessage(session_,writer.write(obj)); 
}

void GatewayClient::onCreateSessionSuccess(const Json::Value& obj)
{
	Json::Value data;
	if (rtc::GetValueFromJsonObject(obj, "data",&data)) {
		Json::Value id = data["id"];
		if (rtc::GetUInt64FromJson(id,&session_id)) {	
			atomic_int_set(&session_activity,1);
			event.Set();
			if (callback_)
				callback_->onSuccess();
		}
	} 	
}

void GatewayClient::attachPluginSuccess(
	PluginType type,const Json::Value& obj)
{
	
}

void GatewayClient::onAskSuccess(
	PluginType type,askInfo* info,const Json::Value& obj)
{
	
}

void GatewayClient::onError(const std::string& error)
{

}

