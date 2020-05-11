#ifndef GATEWAY_CLIENT_H
#define GATEWAY_CLIENT_H

#include "transports/transport.h"
#include "rtc_base/critical_section.h"
#include "rtc_base/strings/json.h"
#include "transaction.h"

class IGatewayCallback {
 public:	
  virtual ~IGatewayCallback() {};
	
  virtual void onSuccess() = 0;

  virtual void onDestroy() = 0;

  virtual void onError(const std::string& error) = 0;	
};

class GatewayClient 
	: public ITransportCallbacks, 
		public IErrorCallbacks,
		public ICreateSessionCallbacks,
		public IAttachPluginCallbacks,
		public IAskCallbacks {
 public:	
	/**
	 *when using client sdk
	 */	
	explicit GatewayClient(IGatewayCallback* callback);
	/**
	 *
	 */
  ~GatewayClient() override ; 	 
	/**
	 *send message when client using
	 */
	void sendMessage(const std::string message);
	/**
	 *attach plugin
	 */	
	//void attach(PluginType type);
	/**
	 *call to Peer
	 */	
	void callP2p(
		const std::string& peerName,const std::string& sdp);

 protected:
	/**
	 * Callback to check with the core if an API secret must be provided
	 */
	bool IsSecretNeeded(gateway_transport* transport) override {};
	/**
	 * Callback to check with the core if an API secret valid
	 */	
	bool IsSecretValid(gateway_transport* transport,const std::string& secret) override {};
	/**
	 * Callback to check with the core if an authtoken nedded
	 */	
	bool IsAuthTokenNeeded(gateway_transport* transport) override {};
	/**
	 * Callback to check with the core if an authtoken valid
	 */		
	bool IsAuthTokenValid(gateway_transport* transport,const std::string& auth_token) override {};
  /**
   * server request
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

	void gatewayInfo();
	/**
	 *@ param transaction
	 */
	const std::string putNewTransaction(
			std::unique_ptr<ITransaction> transaction); 
	/**
	 *@ param session
	 */	
 	void createSession();
  void onCreateSessionSuccess(const Json::Value& obj) override ;
	
	void onAskSuccess(
		PluginType type,askInfo* info,const Json::Value& obj) override ;

 	void attachPluginSuccess(
				 PluginType type,
				 /*IPluginEventListener* listerner,*/
				 const Json::Value& obj) override;
	
 	void onError(const std::string& error) override;
	
 private:
 	std::string token;
	uint64_t session_id;
	int session_activity;
 private: 	
 	rtc::Event event;
 	webrtc::Clock* clock_;
	IGatewayCallback* callback_;	
	gateway_transport_session session_;	
	rtc::CriticalSection transactionsLock;
	rtc::CriticalSection attachedPluginsLock;
	std::unique_ptr<ITransport> transport_;	
	std::unique_ptr<rtc::Thread> gateway_thread_;
	std::map<uint64_t,std::unique_ptr<PluginHandle>> attachedPlugins;
	std::map<std::string,std::unique_ptr<ITransaction>> transactions;	
};
#endif
