
#ifndef GATEWAY_SDK_PLUGIN_H_
#define GATEWAY_SDK_PLUGIN_H_

#include <memory>
#include <string>

class VideoCallEvent {
 public:
 	virtual ~VideoCallEvent() = default;
	/**
	 * @point to plugin
	 */	
	void *p_instance;
	/**
	 * @oncalling
	 */
	virtual void onCalling() = 0;
	/**
	 * @ when incoming call
	 * @ name:peerName
	 */	
	virtual void onIncomingcall(
		const std::string& name,const std::string& remote_jsep) = 0; 
	/**
	 * @ when other peer accepted the call
	 * @ jsep remote sdp
	 */	
	virtual void onCallAccepted(const std::string& remote_jsep) = 0; 
	/**
	 * @when other peer hangup the call
	 */	
	virtual void onCallHangup(const std::string& reason) = 0; 
	/**
	 * @when the peer connected with remote
	 */
	virtual void onCallConnected() = 0;  
};

class PluginHandle {
 public:
  virtual ~PluginHandle() = default;
	
  virtual void startCall(
  	const std::string& peerName,const std::string& jsep) = 0;
	
	virtual void answerCall(const std::string& jsep) = 0;
	
	virtual void hangUp() = 0;    
	
};
#endif
