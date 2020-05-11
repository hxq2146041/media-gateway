#ifndef GATEWAY_P2P_CALL_PLUGIN_H
#define GATEWAY_P2P_CALL_PLUGIN_H

#include "plugin.h"
#include "config.h"
#include "rtc_base/thread.h"

class P2pCall : public Plugin, public rtc::Thread {
 public:
	/*
	 * @params callback:
	 * return cfg_path:config file path
	 */ 	
 	explicit P2pCall(PluginCallbacks* callback);
	/*
	 * @params callback:
	 */ 	
	virtual ~P2pCall();
 	/*
	 * @init p2p call resource
	 */		
 	int init(const std::string& config_path) override;
	/*
	 * @release plugin source and diable it
	 */		
	void destory() override;	 	
	/*
	 *@params out error
	 * return plugin session
	 */
	PluginType getType() override;
	/*
	 *@params out error
	 * return plugin session
	 */
	std::string getPackage() override; 	
	/*
	 *@params out error
	 * return plugin session
	 */
	std::string getName() override;		
	/*
	*@params out error
	* return plugin session
	*/
	gateway_plugin_session createSession(const gateway_handle& handle,int *error)  override; 
	/**
	* destory the session
	*/
	void destorySession(const gateway_plugin_session& session, int *error) override;

	/*! \brief Method to handle an incoming message/request from a peer
	 * @param[in] handle The plugin/gateway session used for this peer
	 * @param[in] transaction The transaction identifier for this message/request
	 * @param[in] message The json_t object containing the message/request JSON
	 * @param[in] jsep The json_t object containing the JSEP type/SDP, if available
	 * @returns A janus_plugin_result instance that may contain a response (for immediate/synchronous replies), an ack
	 * (for asynchronously managed requests) or an error */
	virtual gateway_plugin_result handleMessage(
		const gateway_plugin_session& session, 
		const std::string& transaction, 
		const json_value& message, 
		const json_value& jsep) override;	
	/**
	*return the json string session info
	*/
	std::string querySession(const gateway_plugin_session& session) override; 
	
 protected:
 	class P2pCallSession;
	P2pCallSession* lookupP2pSession(const gateway_plugin_session& session); 
	int processError(
		const gateway_plugin_session& session,
		int error_code,
		const char* error_cause,
		const std::string& transaction);
 	/*
   *  thread run;
	 */
	void Run() override;	
 private:
 	volatile int initialized;
	volatile int stopping;
 	gateway_cfg* config_;
	std::string  config_path_;
	PluginCallbacks* callback_;
	async_queue messages;
	rtc::CriticalSection sessionsLock;
	std::map<std::string,std::string> usernames;	
	std::map<gateway_plugin_session,gateway_plugin_session> sessions;	
};

typedef struct gateway_videocall_message {
	gateway_plugin_session session;
	std::string transaction;
	json_value message;
	json_value jsep;
} gateway_videocall_message;

gateway_videocall_message* gateway_videocall_message_new(
	const gateway_plugin_session& session, 
	const std::string& transaction, 
	const json_value& message, 
	const json_value& jsep);

void gateway_videocall_message_free(gateway_videocall_message *msg); 


#endif
