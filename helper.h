#ifndef GATEWAY_HELPER_H
#define GATEWAY_HELPER_H
#include <string.h>
#include <map>
#include "error.h"

#include "rtc_base/strings/json.h"
#include "api/scoped_refptr.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/ip_address.h"

class Gateway;
class GatewayClient;
class GatewayRequest;
class GatewaySession;
class Handle;
class Plugin;
class PluginSession;
class PluginResult;

#define GATEWAY_JSON_PARAM_REQUIRED	1
#define GATEWAY_JSON_PARAM_POSITIVE	2
#define GATEWAY_JSON_PARAM_NONEMPTY	4

template <typename type,class T>	
using nomal_map = std::map<type,T>;

template <typename type,class T>
using scoped_map = std::map<type,rtc::scoped_refptr<T>>;

template <typename type,class T>	
using unique_map = std::map<type,std::unique_ptr<T>>;

typedef rtc::IPAddress 								network_address;

typedef Json::Value										json_value;
typedef Json::ValueType								json_type;


#define json_string						Json::ValueType::stringValue
#define json_int 							Json::ValueType::intValue
#define json_uint 						Json::ValueType::uintValue
#define json_double 					Json::ValueType::realValue
#define json_bool 						Json::ValueType::booleanValue
#define json_array 						Json::ValueType::arrayValue
#define json_object 					Json::ValueType::objectValue

typedef Gateway gateway;
typedef GatewayClient gateway_client;

typedef rtc::scoped_refptr<GatewaySession> gateway_session;
typedef rtc::scoped_refptr<GatewayRequest> gateway_request;
typedef rtc::scoped_refptr<Handle> 				 gateway_handle;
typedef rtc::scoped_refptr<Plugin> 				 gateway_plugin;
typedef rtc::scoped_refptr<PluginSession>  gateway_plugin_session;
typedef rtc::scoped_refptr<PluginResult> 	 gateway_plugin_result;

enum GatewayType {
	GATEWAY_CREATE,
	GATEWAY_KEEPALIVE,	
	GATEWAY_ASK,	
	GATEWAY_ATTACH,
	GATEWAY_DESTORY,	
	GATEWAY_DETACH,
	GATEWAY_HANGUP,
	GATEWAY_CLAIM,
	GATEWAY_MESSAGE,
	GATEWAY_TRICKLE,
	GATEWAY_EVENT,
	GATEWAY_ERROR,
	GATEWAY_ACK,
	GATEWAY_SUCCESS,
	GATEWAY_WEBRTCUP,
	GATEWAY_DETACHED,
	GATEWAY_MEDIA,
	GATEWAY_SLOWLINK,
	GATEWAY_UNKNOWN
};

enum PluginType {
	GATEWAY_PLUGIN_P2PCALL,
	GATEWAY_PLUGIN_VIDEOCALL,
	GATEWAY_PLUGIN_UNKNOWN
};

const char* gatewayTypeToString(GatewayType type) ;

GatewayType gatewayTypeFromString(const char* type);

const char* pluginTypeToString(PluginType type) ;

PluginType 	pluginTypeFromString(const char* type);

#endif
