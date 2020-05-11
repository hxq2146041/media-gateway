#ifndef MEDIA_GATEWAY_H
#define MEDIA_GATEWAY_H
#include <stdarg.h>
#include <getopt.h>
#include <poll.h>

#include "rtc_base/critical_section.h"
#include "rtc_base/task_queue.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/thread_pool.h"
#include "rtc_base/string_to_number.h"
#include "rtc_base/strings/string_builder.h"
#include "rtc_base/logging.h"
#include "rtc_base/thread.h"
#include "rtc_base/time_utils.h"
#include "system_wrappers/include/clock.h" 
#include "transports/transport.h"
#include "handle/handle.h"
#include "plugins/plugin.h"
#include "utils.h"
#include "helper.h"
#include "cmdline.h"

typedef struct gengetopt_args_info args_info_t;
typedef webrtc::TaskQueueFactory TaskQueueFactory;

class GatewayRequest : public rtc::RefCountInterface {
 public:
 	virtual ~GatewayRequest() = default;	
	virtual bool HasOneRef() const = 0;
	/*point Gateway*/
	void* gateway_p;
	/*! \brief Pointer to the transport plugin */
	gateway_transport *transport;
	/*! \brief Pointer to the transport-provided session instance */
	gateway_transport_session instance;
	/*! \brief Opaque pointer to the request ID, if available */
	void *request_id;
	/*! \brief Whether this is a Gateway API or admin API request */
	bool admin;
	/*! \brief Pointer to the original request, if available */
	char* message;
	/*! \brief Atomic flag to check if this instance has been destroyed */
	volatile int destroyed;	
};
	
class GatewaySession : public rtc::RefCountInterface {
 public:
 	virtual ~GatewaySession() = default;
	virtual bool HasOneRef() const = 0;
	/*point Gateway*/
	void* gateway_p;	
	/*public user id*/
	std::string session_uid;
	/*! \brief Gateway Core-Client session ID */
	uint64_t session_id;
	/*! \brief Map of handles this session is managing */
	std::map<uint64_t,gateway_handle> session_handles; 	
	/*! \brief Time of the last activity on the session */
	int64_t last_activity;
	/*! \brief Pointer to the request instance (and the transport that originated the session) */
	gateway_request source;
	/*! \brief Flag to notify there's been a session timeout */
	volatile int timeout;
	/*! \brief Flag to notify that transport is gone */
	volatile int transport_gone;
	/*! \brief Mutex to lock/unlock this session */
	rtc::CriticalSection mutex;
	/*! \brief Atomic flag to check if this instance has been destroyed */
	volatile int destroyed;	
};

class Gateway : public ITransportCallbacks,
								public PluginCallbacks, 
								public rtc::MessageHandler ,
								public sigslot::has_slots<> {
 public:	
  Gateway(); 	
  ~Gateway() override ; 

	static int StartArgs(args_info_t& args_info);

	json_value gatewayInfo(const std::string& transaction);

	rtc::ThreadPool* thread_pool() {return thread_pool_.get();}

	gateway_handle gatewaySessionFindHandles(gateway_session& session, uint64_t handle_id); 
	void gatewaySessionInsertHandles(gateway_session& session, gateway_handle& handle); 
	int gatewaySessionRemoveHandles(gateway_session& session, gateway_handle& handle);
	void gatewaySessionClearHandles(gateway_session& session);
 	
 protected:
 	class LogFileSink;
	class RequestTask;	
 	/***********************implement ITransportCallbacks**************************/
	/**
	 * Callback to check with the core if an API secret must be provided
	 */
	bool IsSecretNeeded(gateway_transport* transport) override ;
	/**
	 * Callback to check with the core if an API secret valid
	 */	
	bool IsSecretValid(gateway_transport* transport,const std::string& secret) override ;
	/**
	 * Callback to check with the core if an authtoken nedded
	 */	
	bool IsAuthTokenNeeded(gateway_transport* transport) override ;
	/**
	 * Callback to check with the core if an authtoken valid
	 */		
	bool IsAuthTokenValid(gateway_transport* transport,const std::string& auth_token) override ;
  /**
   * request
   */
  void IncomingRequest(
  	gateway_transport* transport,
  	gateway_transport_session& session,
  	void *request_id,bool admin,char* message) override ;
  /**
   * gone
   */
  void TransportGone(
  	gateway_transport* transport,
  	gateway_transport_session& session) override ;
  /**
   * @param session: gateway session
   * @param event : jsson event message
   */
  void NotifyEvent(
  	gateway_transport* transport,
  	gateway_transport_session& session,
  	const std::string& event) override ;
 
	/***********************implement plugin callbacks**************************/
	int sendEvent(
		const gateway_plugin& plugin,
		const gateway_plugin_session& session,
		const std::string& transaction,
		const json_value& message,
		const json_value& jsep) override;

  virtual void colsePc(
	  const gateway_plugin_session& session) override;  

	
 protected:
 	/***********************implement message handle**************************/
  void OnMessage(rtc::Message* message);
	void checkSessions(); 
	void destorySessions(); 
	void createPlugins();
	void destoryPlugins();

 protected:
	/***********************implement gateway_request**************************/
	gateway_request gatewayRequestNew(
		 gateway_transport* transport, 
		 gateway_transport_session& instance, 
		 void *request_id, bool admin, char* message);
	void gatewayRequestDestory(gateway_request& request);
	uint64_t gatewaySessionIdFind(const std::string& session_uid);
	gateway_session gatewaySessionFind(uint64_t session_id);
	gateway_session gatewaySessionCreate(
		uint64_t session_id,const std::string& session_uid);
	int gatewaySessionDestory(gateway_session& session);
	gateway_request gatewaySessionRequest(gateway_session& session);

	gateway_plugin gatewayPluginFind(const std::string& plugin);
	
	json_value createGatewayMessage(
		const std::string& status, uint64_t session_id, const std::string& transaction); 
	
	int requestProcessSuccess(gateway_request& request,const Json::Value& payload);

	int requestProcessErrorString(
		gateway_request& request, uint64_t session_id, 
		const std::string& transaction, int error, char* error_string);
	
	int requestProcessError(
		gateway_request& request, uint64_t session_id, 
		const std::string& transaction, int error, const char *format, ...);

	static void requestsProcess(gateway_request request);	
	
	int requestBaseProcess(gateway_request& request,const Json::Value& root);
	
	int requestAttachProcess(gateway_session& session,gateway_request& request,const Json::Value& root);

 	int requestProcess(gateway_request request);

 	int requestAdminProcess(gateway_request request);

	void parseCommonConfig();
	
	void parseNiceConfig();

 private:
	webrtc::Clock* clock_;
 private: 	
 	rtc::TaskQueue request_queue_;
	rtc::CriticalSection transportsLock;
	rtc::CriticalSection sessionsLock;
	rtc::CriticalSection pluginsLock;	
	std::unique_ptr<rtc::ThreadPool> thread_pool_;
	std::map<uint64_t,gateway_session> sessions_;
	std::map<std::string,uint64_t> uid_sessions;
	std::unique_ptr<rtc::Thread> session_watchdog_;	
	std::unique_ptr<rtc::Thread> notify_thread_;	
	std::map<std::string,gateway_plugin> plugins_;	
	std::map<TransportType,
				std::unique_ptr<ITransport>> transports_;
};

#endif
