#ifndef GATEWAY_P2P_HANDLE_H
#define GATEWAY_P2P_HANDLE_H

#include "handle.h"

class P2pHandle : public Handle {
 public:
 	explicit P2pHandle(
		gateway_session& core_session,const std::string opaque_id);
  virtual ~P2pHandle();
protected:
	 /**
		* 
		*/ 
	 HandleType getType() override;	 
	 /**
		* attach plugin
		*/ 
	 int createSession(const gateway_plugin& plugin) override;	 
	 /**
		* create NiceAgent *
		*/ 
	 int destorySession() override;	 
	 /**
		* create NiceAgent *
		*/ 
	 void createAgent(int offer) override {};
};
#endif
