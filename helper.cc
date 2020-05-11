#include "helper.h"

static const char gateway_message[] = "message";
static const char gateway_trickle[] = "trickle";
static const char gateway_detach[] = "detach";
static const char gateway_destory[] = "destory";
static const char gateway_keepalive[] = "keepalive";
static const char gateway_create[] = "create";
static const char gateway_attach[] = "attach";
static const char gateway_ask[] = "ask";
static const char gateway_event[] = "event";
static const char gateway_error[] = "error";
static const char gateway_ack[] = "ack";
static const char gateway_success[] = "success";
static const char gateway_webrtcup[] = "webrtcup";
static const char gateway_hangup[] = "hangup";
static const char gateway_detached[] = "detached";
static const char gateway_media[] = "media";
static const char gateway_slowlink[] = "slowlink";
static const char gateway_claim[] = "claim";



static const char gateway_unknown[] = "";

const char* gatewayTypeToString(GatewayType type) 
{
  switch (type) {
    case GATEWAY_MESSAGE:
      return gateway_message;
    case GATEWAY_TRICKLE:
      return gateway_trickle;
    case GATEWAY_DETACH:
      return gateway_detach;
    case GATEWAY_DESTORY:
      return gateway_destory;
    case GATEWAY_KEEPALIVE:
      return gateway_keepalive;
    case GATEWAY_CREATE:
      return gateway_create;	
    case GATEWAY_ATTACH:
      return gateway_attach;	
    case GATEWAY_ASK:
      return gateway_ask;			
    case GATEWAY_EVENT:
      return gateway_event;	
    case GATEWAY_ERROR:
      return gateway_error;
		case GATEWAY_ACK:
      return gateway_ack;	
    case GATEWAY_SUCCESS:
      return gateway_success;
		case GATEWAY_WEBRTCUP:
      return gateway_webrtcup;	
    case GATEWAY_HANGUP:
      return gateway_hangup;
		case GATEWAY_DETACHED:
      return gateway_detached;	
    case GATEWAY_MEDIA:
      return gateway_media;	
    case GATEWAY_SLOWLINK:
      return gateway_slowlink;			
		default:
			return gateway_unknown;
  }
  return "";
}

GatewayType gatewayTypeFromString(const char* type) 
{
	if (!strcasecmp(gateway_message,type)) 
		return GATEWAY_MESSAGE;		
	else if (!strcasecmp(gateway_trickle,type))
		return GATEWAY_TRICKLE; 	
	else if (!strcasecmp(gateway_detach,type))
		return GATEWAY_DETACH; 
	else if (!strcasecmp(gateway_keepalive,type))
		return GATEWAY_KEEPALIVE; 	
	else if (!strcasecmp(gateway_destory,type))
		return GATEWAY_DESTORY; 
	else if (!strcasecmp(gateway_create,type))
		return GATEWAY_CREATE; 
	else if (!strcasecmp(gateway_attach,type))
		return GATEWAY_ATTACH; 
	else if (!strcasecmp(gateway_ask,type))
		return GATEWAY_ASK; 
	else if (!strcasecmp(gateway_event,type))
		return GATEWAY_EVENT; 
	else if (!strcasecmp(gateway_error,type))
		return GATEWAY_ERROR; 
	else if (!strcasecmp(gateway_ack,type))
		return GATEWAY_ACK; 
	else if (!strcasecmp(gateway_success,type))
		return GATEWAY_SUCCESS; 
	else if (!strcasecmp(gateway_webrtcup,type))
		return GATEWAY_WEBRTCUP; 
	else if (!strcasecmp(gateway_hangup,type))
		return GATEWAY_HANGUP; 
	else if (!strcasecmp(gateway_detached,type))
		return GATEWAY_DETACHED; 
	else if (!strcasecmp(gateway_media,type))
		return GATEWAY_MEDIA; 
	else if (!strcasecmp(gateway_slowlink,type))
		return GATEWAY_SLOWLINK; 		
	else 
		return GATEWAY_UNKNOWN; 
}

static const char plugin_type_p2p_call[] = "gateway.plugin.p2pcall";
static const char plugin_type_video_call[] = "gateway.plugin.videocall";

const char* pluginTypeToString(PluginType type)
{
  switch (type) {
    case GATEWAY_PLUGIN_P2PCALL:
      return plugin_type_p2p_call;
    case GATEWAY_PLUGIN_VIDEOCALL:
      return plugin_type_video_call;
		default:
			return gateway_unknown;
  }
  return "";
}

PluginType pluginTypeFromString(const char* type)
{
	if (!strcasecmp(plugin_type_p2p_call,type)) 
		return GATEWAY_PLUGIN_P2PCALL;		
	else if (!strcasecmp(plugin_type_video_call,type))
		return GATEWAY_PLUGIN_VIDEOCALL; 	
	else 
		return GATEWAY_PLUGIN_UNKNOWN; 
}

