#ifndef GATEWAY_PLUGIN_H
#define GATEWAY_PLUGIN_H

#include <string>
#include "rtc_base/strings/json.h"
#include "rtc_base/ref_count.h"
#include "async_queue.h"
#include "helper.h"

enum gateway_plugin_result_type {
	/*! \brief A severe error happened (not an application level error) */
	GATEWAY_PLUGIN_ERROR = -1,
	/*! \brief The request was correctly handled and a response is provided (synchronous) */
	GATEWAY_PLUGIN_OK,
	/*! \brief The request was correctly handled and notifications will follow with more info (asynchronous) */
	GATEWAY_PLUGIN_OK_WAIT,
} ;

class PluginResult : public rtc::RefCountInterface {
 public:
	virtual ~PluginResult() = default;
	virtual bool HasOneRef() const = 0; 	
	/*! \brief Result type */
	gateway_plugin_result_type type;
	/*! \brief Text associated with this plugin result.
	* @note This is ONLY used for GATEWAY_PLUGIN_OK_WAIT (to provide hints on
	* why a request is being handled asynchronously) and GATEWAY_PLUGIN_ERROR
	* (to provide a reason for the error). It is ignored for GATEWAY_PLUGIN_OK.
	* Besides, it is NOT freed when destroying the janus_plugin_result instance,
	* so if you allocated a string for that, you'll have to free it yourself. */
	std::string text;
	/*! \brief Result content
	* @note This is ONLY used for GATEWAY_PLUGIN_OK, and is ignored otherwise.
	* It MUST be a valid JSON payload (even when returning application
	* level errors). Its reference is decremented automatically when destroying
	* the janus_plugin_result instance, so if your plugin wants to re-use the
	* same object for multiple responses, you jave to \c json_incref the object before
	* passing it to the core, and \c json_decref it when you're done with it. */
	json_value content; 	
};

gateway_plugin_result gateway_plugin_result_new(
	gateway_plugin_result_type type, 
	const std::string& text, 
	const json_value& content); 

void gateway_plugin_result_destroy(gateway_plugin_result& result) ;

/*! \brief Plugin-Gateway session mapping */
class PluginSession : public rtc::RefCountInterface {
 public: 
	virtual ~PluginSession() = default;
	virtual bool HasOneRef() const = 0;
	/*mark this active in active map*/
	uint64_t session_id;
	/*! \brief Opaque pointer to the Gateway core core-level handle */
	gateway_handle g_handle;
	/*! \brief Opaque pointer to the plugin session */
	//void *plugin_handle;
	/*! \brief Whether this mapping has been stopped definitely or not: if so,
	 * the plugin shouldn't make use of it anymore */
	volatile int stopped;
};

class Plugin : public rtc::RefCountInterface {
 public:
	virtual ~Plugin() = default;
	
	virtual bool HasOneRef() const = 0; 	
 	/*
	 * @init implement plugin resource
	 */		
 	virtual int init(const std::string& config_path) = 0;
	/*
	 * @release plugin source and diable it
	 */		
	virtual void destory() = 0;		
	/*
	 *@params out error
	 * return plugin session
	 */
	virtual PluginType getType() = 0;
	/*
	 *@params out error
	 * return plugin session
	 */
	virtual std::string getPackage() = 0;

	/*
	 *@params out error
	 * return plugin session
	 */
	virtual std::string getName() = 0;	
	/*
	 *@params out error
	 * return plugin session
	 */
	virtual gateway_plugin_session createSession(const gateway_handle& handle,int *error) = 0;	
	/**
	 * destory the session
	 */
	virtual void destorySession(const gateway_plugin_session& session, int *error) = 0;

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
		const json_value& jsep) = 0;

	/**
	 *return the json string session info
	 */
	virtual std::string querySession(const gateway_plugin_session& session) = 0;
	
};


class PluginCallbacks {
 public:
	virtual ~PluginCallbacks() = default;
	/**
	 *@params: plugin which plugin? 
	 *@session: which plugin which session?
	 *@message:message
	 *@jsep:
	 */
	virtual int sendEvent(
		const gateway_plugin& plugin,
		const gateway_plugin_session& session,
		const std::string& transaction,
		const json_value& message,
		const json_value& jsep) = 0;
	/*! \brief Callback to ask the core to close a WebRTC PeerConnection
	 * \note A call to this method will result in the core invoking the hangup_media
	 * callback on this plugin when done
	 * @param[in] handle The plugin/gateway session that the PeerConnection is related to */	
	virtual void colsePc(
		const gateway_plugin_session& session) = 0;
};

gateway_plugin createPlugin(
	PluginCallbacks* callback,PluginType type/*,const std::string& cfg*/);


#endif
