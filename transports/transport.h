#ifndef GATEWAY_TRANSPORT_H
#define GATEWAY_TRANSPORT_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <inttypes.h>
#include <string>
#include <map>
#include "config.h"
#include "mutex.h"
#include "atomic.h"
#include "utils.h"
#include "async_queue.h"
#include "rtc_base/thread.h"
#include "rtc_base/null_socket_server.h"
#include "rtc_base/logging.h"
#include "api/scoped_refptr.h" 
#include "rtc_base/ref_counted_object.h"

typedef struct transport_message transport_message;
struct transport_message {
	char* msg;
};

void transport_message_queue_init(async_queue *mq,int capacity);
void transport_message_queue_clean(async_queue *mq);

async_queue_element *transport_message_new(async_queue *mq); 
void transport_message_free(async_queue *mq,async_queue_element *item); 

void transport_message_push(async_queue *mq, async_queue_element *item);
async_queue_element* transport_message_pop(async_queue *mq);

/*! \brief Transport-Gateway session mapping */
class GatewayTransportSession : public rtc::RefCountInterface {
 public:
 	virtual ~GatewayTransportSession() = default;	
	virtual bool HasOneRef() const = 0;
	/*! \brief Opaque pointer to the transport session 
		point to 
							1.websockets_transport_client
							2.http_transport_client
							3.socket_transport_client*/
	void *transport_p;
	/*! \brief Pointer to the transport-provided function, if needed, that will be used to free the opaque transport session instance */
	void (*p_free)(void *);	
	/*! \brief Whether this mapping has been destroyed definitely or not: if so,
	 * the transport shouldn't make use of it anymore */
	volatile int destroyed;	
	/*! \brief Mutex to protect changes to transport_p */
	gateway_mutex mutex;
};
typedef rtc::scoped_refptr<GatewayTransportSession> gateway_transport_session;


/*! \brief Helper to create a gateway_transport_session instance
 * @note This helper automatically initializes the reference counter
 * @param transport_p Pointer to the transport-side session instance (won't be touched by the core)
 * @param p_free Pointer to the transport-provided function, if needed, that will be used to free the opaque transport-side session instance (won't be touched by the core)
 * @returns Pointer to a valid gateway_transport_session, if successful, NULL otherwise */
gateway_transport_session gateway_transport_session_create(void *transport_p, void (*p_free)(void *));
/*! \brief Helper to mark a gateway_transport_session instance as destroyed
 * @note Only use this helper when that specific transport session must not be
 * used by the core anymore: e.g., a WebSocket connection was closed, an
 * HTTP connection associated with a pending request was lost, etc. Remember
 * to decrease the counter in case you increased it in other methods (this
 * method does this automatically as far as the create was concerned).
 * @param session Pointer to the gateway_transport_session instance */
void gateway_transport_session_destroy(gateway_transport_session& session);

/**
 * Created by Jeffrey on 1/19/2018.
 */
enum TransportType {
  /**
   * websocket support
  */
  TRANSPORT_WEBSOCKET,
  /**
   * resetful support
   */ 
  TRANSPORT_RESETFUL,
	/**
	 * LINUX SOCKET
	 */ 
	TRANSPORT_MQTT,

  /**
   * LINUX SOCKET
   */ 
  TRANSPORT_USOCKET	
};


/**
 * Created by Jeffrey on 8/10/2019.
 */
class ITransport;  
typedef ITransport gateway_transport; 
class ITransport {
 public:	
  virtual ~ITransport() = default;
	/**
   * connect to server or listen when using client
   */
	virtual int connect(const std::string url,int port) = 0; 
  /**
   * connect to server or listen when using server
   */
  virtual int connect(std::string cfg_path) = 0;
  /**
   * disconnect the server or client
   */
  virtual int disconnect() = 0;	
  /**
   * delete
   */
	virtual void destory() = 0;
  /**
   * @param message
   * @param message
   */
  virtual int sendMessage(
  	gateway_transport_session session,const std::string& message) = 0;	
  /**
   * @param session
   * @param request_id
	 * @param admin
	 * @param message
   */
  virtual int sendMessage(
  	gateway_transport_session session,
  	void *request_id, bool admin, const std::string& message) = 0;
  /**
   * @param session
   * @param session_id
   * when gateway session created
   */
	virtual void sessionCreated(
		gateway_transport_session session,uint64_t session_id) = 0;
  /**
   * @param session
   * @param session_id
   * when gateway session Over
   */
	virtual void sessionOver(
		gateway_transport_session session,
		uint64_t session_id, bool timeout, bool claimed) = 0;
  /**
   * @param session
   * @param session_id
   * when gateway session Claimed
   */
	virtual void sessionClaimed(
		gateway_transport_session session,uint64_t session_id) = 0;
  /**
   * return the transport type
   */
  virtual TransportType getType() = 0;
  /**
   * @return transport module package
   */
	virtual std::string getPackage() = 0;
};

class ITransportCallbacks {
 public:	
  virtual ~ITransportCallbacks() {}
	/**
	 * Callback to check with the core if an API secret must be provided
	 */
	virtual bool IsSecretNeeded(gateway_transport* transport) = 0;
	/**
	 * Callback to check with the core if an API secret valid
	 */	
	virtual bool IsSecretValid(
		gateway_transport* transport,const std::string& secret) = 0;
	/**
	 * Callback to check with the core if an authtoken nedded
	 */	
	virtual bool IsAuthTokenNeeded(gateway_transport* transport) = 0;
	/**
	 * Callback to check with the core if an authtoken valid
	 */		
	virtual bool IsAuthTokenValid(
		gateway_transport* transport,const std::string& auth_token) = 0;
  /**
   * request
   */
  virtual void IncomingRequest(
  	gateway_transport* transport,
  	gateway_transport_session& session,
		void *request_id, bool admin,char* message) = 0;
  /**
   * gone
   */
  virtual void TransportGone(
  	gateway_transport* transport,
  	gateway_transport_session& session) = 0;
  /**
   * @param session: gateway session
   * @param event : jsson event message
   */
  virtual void NotifyEvent(
  	gateway_transport* transport,
  	gateway_transport_session& session,
  	const std::string& message) = 0;
};

std::unique_ptr<ITransport> 
	createTransport(TransportType type,ITransportCallbacks *callback);


#endif
