
#ifndef GATEWAY_TRANSPORT_WEBSOCKET_H
#define GATEWAY_TRANSPORT_WEBSOCKET_H

#include "transport.h"
#include <libwebsockets.h>

typedef struct websockets_transport_session websockets_transport_session;
/* WebSocket connection session */
struct websockets_transport_session {
	/*when server is admin?*/
	bool admin;
	/* The libwebsockets conn instance */
	struct lws *wsi;
	/* Queue of outgoing messages to push */
	async_queue messages;
	/* Buffer containing the incoming message to process (in case there are fragments) */
	char* incoming;
	/* Buffer containing the message to send */
	unsigned char *buffer;	
	/* Length of the buffer (may be resized after re-allocations) */
	int buflen;	
	/* Data an interrupted previous write couldn't send */
	int bufpending;
	/* Offset from where the interrupted previous write should resume */
	int bufoffset;	
	/* Whether this libwebsockets client instance has been closed */
	volatile int destroyed;
	/* Gateway core-transport session */
	gateway_transport_session ts;			
};

class WebsocketsCallback {
 public:
  virtual ~WebsocketsCallback() {}		

  virtual void onConnectedCallback(websockets_transport_session* session) = 0;
	
	virtual void onReceivedCallback(websockets_transport_session* session) = 0;	

#if (LWS_LIBRARY_VERSION_MAJOR >= 3)	
	virtual void onWaitCancelledCallback(websockets_transport_session* session) = 0;	
#endif
	virtual void onWriteableCallback(websockets_transport_session* session) = 0;	

  virtual void onDestroyCallback(websockets_transport_session* session) = 0; 	
};

/*server info context*/
typedef struct ws_server_context ws_server_context;
struct ws_server_context {
	/*server ws listen*/
	struct lws_vhost *wss;
	/*server wss listen*/
	struct lws_vhost *swss;
	/*server ws admin listen*/
	struct lws_vhost *admin_wss;
	/*server wss admin listen*/
	struct lws_vhost *admin_swss;
};

/*client info context*/
typedef struct ws_client_context ws_client_context;
struct ws_client_context{
	/*client ws listen*/
	struct lws *wss;
};

/*ws conext server or client*/
typedef struct ws_context ws_context;
struct ws_context {
	bool client;
 	/*ws log level*/
	int ws_log_level;	
	/*ws context*/
	struct lws_context *wsc;
	/*server context*/
	ws_server_context s_context;
	/*client context*/
	ws_client_context c_context;
};

class WebsocketsTransport
	: public ITransport,
		public WebsocketsCallback, 
		public rtc::Thread {
 public:	
  explicit WebsocketsTransport(ITransportCallbacks *callback); 	
	
  ~WebsocketsTransport() override ;
  /**
   * connect to server or listen when using client
   */	
	int connect(const std::string url,int port) override;
  /**
   * connect to server or listen when using server
   */
  int connect(std::string cfg_path) override;
  /**
   * disconnect the server or client
   */
  int disconnect() override;	
  /**
   * delete
   */
	void destory() override;
  /**
   * @param message
   * @param message
   */
  int sendMessage(
  	gateway_transport_session session,const std::string& message) override;	
  /**
   * @param session
   * @param request_id
	 * @param admin
	 * @param message
   */
  int sendMessage(
  	gateway_transport_session session,
  	void *request_id, bool admin, const std::string& message) override;
  /**
   * @param session
   * @param session_id
   * when gateway session created
   */
	void sessionCreated(
		gateway_transport_session session,uint64_t session_id) override {};
  /**
   * @param session
   * @param session_id
   * when gateway session Over
   */
	void sessionOver(
		gateway_transport_session session,
		uint64_t session_id, bool timeout, bool claimed) override {};
  /**
   * @param session
   * @param session_id
   * when gateway session Claimed
   */
	void sessionClaimed(
		gateway_transport_session session,uint64_t session_id) override {};
  /**
   * return the transport type
   */
  TransportType getType() override;
  /**
   * @return transport module package
   */
	std::string getPackage() override;

protected:
  /**
   * @param 
   */		
	int connect();
  /**
   * @param 
   */	
	bool checkAllowed(const std::string& ipaddress, bool admin);
  /**
   * @param client
   */	
  void onConnectedCallback(websockets_transport_session* session) override;
  /**
   * @param client
   */		
	void onReceivedCallback(websockets_transport_session* session) override;	
#if (LWS_LIBRARY_VERSION_MAJOR >= 3)
	/**
   * @param client
   */		
	void onWaitCancelledCallback(websockets_transport_session* session) override;	
#endif

	void onWriteableCallback(websockets_transport_session* session) override;	
	/**
   * @param client
   */	
  void onDestroyCallback(websockets_transport_session* session) override;	
	/*
   *  thread run;
	 */
	void Run() override;	 
 private:
 	std::string cfg_path_;//
 	gateway_cfg* config_;
	ITransportCallbacks *callback_;
#if (LWS_LIBRARY_VERSION_MAJOR >= 3)	
	rtc::CriticalSection sessionsLock;	
	std::map<websockets_transport_session*,
			websockets_transport_session*> sessions_;
	std::map<websockets_transport_session*,
			websockets_transport_session*> writable_sessions_;	
#endif	
 private:
	int initialized;
 	int stopping; 
	ws_context ws_context_;
	rtc::CriticalSection access_list_lock_;
	std::vector<std::string> ws_access_list_;
	std::vector<std::string> ws_admin_access_list_;	
};

#endif
