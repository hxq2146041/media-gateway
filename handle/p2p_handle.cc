#include "p2p_handle.h"
#include "gateway.h"

P2pHandle::P2pHandle(
	gateway_session& core_session,const std::string opaque_id)
	: Handle(core_session,opaque_id)
{

}


P2pHandle::~P2pHandle()
{
	RTC_LOG(INFO) << "start:" << __FUNCTION__;
}

/**
 * 
 */ 
HandleType P2pHandle::getType()
{
	return GATEWAY_HANDLE_P2P;
}

/**
 * attach plugin
 */ 
int P2pHandle::createSession(const gateway_plugin& plugin)
{
	RTC_LOG(INFO) << "start:" << __FUNCTION__;
	return Handle::createSession(plugin);
}

int P2pHandle::destorySession()
{
	return Handle::destorySession();
}


