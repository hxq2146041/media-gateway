#include "error.h"

const char *gateway_api_error(int error) {
	switch(error) {
		case GATEWAY_OK:
			return "Success";
		case GATEWAY_ERROR_UNAUTHORIZED:
			return "Unauthorized request (wrong or missing secret/token)";
		case GATEWAY_ERROR_UNAUTHORIZED_PLUGIN:
			return "Unauthorized access to plugin (token is not allowed to)";
		case GATEWAY_ERROR_UNKNOWN:
			return "Unknown error";
		case GATEWAY_ERROR_TRANSPORT_SPECIFIC:
			return "Transport specific error";
		case GATEWAY_ERROR_MISSING_REQUEST:
			return "Missing request";
		case GATEWAY_ERROR_UNKNOWN_REQUEST:
			return "Unknown request";
		case GATEWAY_ERROR_INVALID_JSON:
			return "Invalid JSON";
		case GATEWAY_ERROR_INVALID_JSON_OBJECT:
			return "Invalid JSON Object";
		case GATEWAY_ERROR_MISSING_MANDATORY_ELEMENT:
			return "Missing mandatory element";
		case GATEWAY_ERROR_INVALID_REQUEST_PATH:
			return "Invalid path for this request";
		case GATEWAY_ERROR_SESSION_NOT_FOUND:
			return "Session not found";
		case GATEWAY_ERROR_HANDLE_NOT_FOUND:
			return "Handle not found";
		case GATEWAY_ERROR_PLUGIN_NOT_FOUND:
			return "Plugin not found";
		case GATEWAY_ERROR_PLUGIN_ATTACH:
			return "Error attaching plugin";
		case GATEWAY_ERROR_PLUGIN_MESSAGE:
			return "Error sending message to plugin";
		case GATEWAY_ERROR_PLUGIN_DETACH:
			return "Error detaching from plugin";
		case GATEWAY_ERROR_JSEP_UNKNOWN_TYPE:
			return "Unsupported JSEP type";
		case GATEWAY_ERROR_JSEP_INVALID_SDP:
			return "Invalid SDP";
		case GATEWAY_ERROR_TRICKE_INVALID_STREAM:
			return "Invalid stream";
		case GATEWAY_ERROR_INVALID_ELEMENT_TYPE:
			return "Invalid element type";
		case GATEWAY_ERROR_SESSION_CONFLICT:
			return "Session ID already in use";
		case GATEWAY_ERROR_UNEXPECTED_ANSWER:
			return "Unexpected ANSWER (no OFFER)";
		case GATEWAY_ERROR_TOKEN_NOT_FOUND:
			return "Token not found";
		case GATEWAY_ERROR_WEBRTC_STATE:
			return "Wrong WebRTC state";
		case GATEWAY_ERROR_NOT_ACCEPTING_SESSIONS:
			return "Currently not accepting new sessions";
		case GATEWAY_ERROR_ASK_TYPE_NOT_FOUND:
			return "ask type not found";
		case GATEWAY_ERROR_ASK_TYPE_EMPTY:
			return "ask type can not empty";
		case GATEWAY_ERROR_ASK_TYPE_ERROR:
			return "ask type not support just call or join";
		case GATEWAY_ERROR_ASK_PARAMS_ERROR:
			return "ask peer params error";

		case GATEWAY_ERROR_ASK_PEER_NOT_ONLINE:
			return "peer not online";			
			
			
		default:
			return "Unknown error";
	}
}
