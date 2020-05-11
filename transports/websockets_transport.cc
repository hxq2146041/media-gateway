#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>

#include "websockets_transport.h"
#include "absl/strings/str_split.h"
#include "rtc_base/strings/json.h"
#include "atomic.h"

/* Transport plugin information */
#define GATEWAY_WEBSOCKETS_VERSION			1
#define GATEWAY_WEBSOCKETS_VERSION_STRING		"0.0.1"
#define GATEWAY_WEBSOCKETS_DESCRIPTION			"This transport plugin adds WebSockets support to the Gateway API via libwebsockets."
#define GATEWAY_WEBSOCKETS_NAME							"GATEWAY WebSockets transport plugin"
#define GATEWAY_WEBSOCKETS_AUTHOR						"gateway"
#define GATEWAY_WEBSOCKETS_PACKAGE					"gateway.transport.websockets"


static const char* token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9";

static WebsocketsCallback* event_callback = NULL;

/* Helper method to return the interface associated with a local IP address */
static char *websockets_transport_get_interface_name(const char *ip) {
	struct ifaddrs *addrs = NULL, *iap = NULL;
	if(getifaddrs(&addrs) == -1)
		return NULL;
	for(iap = addrs; iap != NULL; iap = iap->ifa_next) {
		if(iap->ifa_addr && (iap->ifa_flags & IFF_UP)) {
			if(iap->ifa_addr->sa_family == AF_INET) {
				struct sockaddr_in *sa = (struct sockaddr_in *)(iap->ifa_addr);
				char buffer[16];
				inet_ntop(iap->ifa_addr->sa_family, (void *)&(sa->sin_addr), buffer, sizeof(buffer));
				if(!strcmp(ip, buffer)) {
					char *iface = strdup(iap->ifa_name);
					freeifaddrs(addrs);
					return iface;
				}
			} else if(iap->ifa_addr->sa_family == AF_INET6) {
				struct sockaddr_in6 *sa = (struct sockaddr_in6 *)(iap->ifa_addr);
				char buffer[48];
				inet_ntop(iap->ifa_addr->sa_family, (void *)&(sa->sin6_addr), buffer, sizeof(buffer));
				if(!strcmp(ip, buffer)) {
					char *iface = strdup(iap->ifa_name);
					freeifaddrs(addrs);
					return iface;
				}
			}
		}
	}
	freeifaddrs(addrs);
	return NULL;
}

/* Logging */
static const char *websockets_transport_get_level_str(int level) {
	switch(level) {
		case LLL_ERR:
			return "ERR";
		case LLL_WARN:
			return "WARN";
		case LLL_NOTICE:
			return "NOTICE";
		case LLL_INFO:
			return "INFO";
		case LLL_DEBUG:
			return "DEBUG";
		case LLL_PARSER:
			return "PARSER";
		case LLL_HEADER:
			return "HEADER";
		case LLL_EXT:
			return "EXT";
		case LLL_CLIENT:
			return "CLIENT";
		case LLL_LATENCY:
			return "LATENCY";
#if (LWS_LIBRARY_VERSION_MAJOR >= 2 && LWS_LIBRARY_VERSION_MINOR >= 2) || (LWS_LIBRARY_VERSION_MAJOR >= 3)
		case LLL_USER:
			return "USER";
#endif
		case LLL_COUNT:
			return "COUNT";
		default:
			return NULL;
	}
}

/* This callback handles Gateway API requests */
static int websockets_transport_common_callback(
		struct lws *wsi,
		enum lws_callback_reasons reason,
		void *user, void *in, size_t len, bool admin)
{
	const char *log_prefix = admin ? "AdminWSS" : "WSS";
	websockets_transport_session *ws_conn = (websockets_transport_session *)user;
	switch(reason) {
		#if 0
		/*server*/
		case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
		case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: {
			RTC_LOG(LS_VERBOSE) << log_prefix << "-- LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION";
			int length = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_AUTHORIZATION);
			char dst[strlen(token) + 2]; 
			lws_hdr_copy(wsi, dst, length + 1, WSI_TOKEN_HTTP_AUTHORIZATION);
			std::string info = dst;
			RTC_LOG(LS_VERBOSE) << log_prefix << "-- token: " << info;
			return 0;
		}
		/*client*/
		case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: {
			RTC_LOG(LS_VERBOSE) << log_prefix << "-- append handshake header";
			unsigned char **p = (unsigned char **)in, *end = (*p) + len;
			#if 0
			if (lws_add_http_header_by_token(wsi,
					WSI_TOKEN_HTTP_CONTENT_LENGTH,
					(unsigned char *)"29", 2, p, end))
				return -1;
			if (lws_add_http_header_by_token(wsi,
					WSI_TOKEN_HTTP_CONTENT_TYPE,
					(unsigned char *)"application/x-www-form-urlencoded",
					33, p, end))
				return -1;
			#endif
			if (lws_add_http_header_by_token(wsi,
					WSI_TOKEN_HTTP_AUTHORIZATION,
					(unsigned char *)token,strlen(token), p, end))
					return -1;
			#if 0
			/* inform lws we have http body to send */
			lws_client_http_body_pending(wsi, 1);
			lws_callback_on_writable(wsi);
			#endif
			
			return 0;
		}
		
		#endif
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
		case LWS_CALLBACK_ESTABLISHED: {
			/* Prepare the session */
			ws_conn->admin = admin;
			ws_conn->wsi = wsi;
			ws_conn->incoming = NULL;
			ws_conn->buffer = NULL;
			ws_conn->buflen = 0;
			ws_conn->bufpending = 0;
			ws_conn->bufoffset = 0;
			atomic_int_set(&ws_conn->destroyed, 0);
			if (event_callback)
				event_callback->onConnectedCallback(ws_conn);
			return 0;
		}
		
		case LWS_CALLBACK_CLIENT_RECEIVE:
		case LWS_CALLBACK_RECEIVE: {
			RTC_LOG(LS_VERBOSE) << log_prefix << " Got " << len << " bytes";
			if(ws_conn == NULL || ws_conn->wsi == NULL) {
				RTC_LOG(LS_VERBOSE) << log_prefix << " Invalid WebSocket client instance... ";
				return -1;
			}
			if(atomic_int_get(&ws_conn->destroyed))
				return 0;
			/* Is this a new message, or part of a fragmented one? */
			const size_t remaining = lws_remaining_packet_payload(wsi);
			if(ws_conn->incoming == NULL) {
				RTC_LOG(LS_VERBOSE) << log_prefix << " First fragment: " 
														<< len << " bytes " << remaining << " remaining";
				ws_conn->incoming = (char *)malloc(len+1);
				memcpy(ws_conn->incoming, in, len);
				ws_conn->incoming[len] = '\0';
			} else {
				size_t offset = strlen(ws_conn->incoming);
				RTC_LOG(LS_VERBOSE) << log_prefix << " Appending fragment: " 
														<< " offset " << offset << " " << len << " bytes "
														<< remaining << remaining;
				memcpy(ws_conn->incoming+offset, in, len);
				ws_conn->incoming[offset+len] = '\0';
			}
			
			if(remaining > 0 || !lws_is_final_fragment(wsi)) {
				/* Still waiting for some more fragments */
				RTC_LOG(LS_VERBOSE) << log_prefix << " Waiting for more fragments ";				
				return 0;
			}
			
			if (event_callback)
				event_callback->onReceivedCallback(ws_conn);
			
			if (ws_conn->incoming != NULL )
				free(ws_conn->incoming);
			ws_conn->incoming = NULL;	

			return 0;
		}
#if (LWS_LIBRARY_VERSION_MAJOR >= 3)
		/* On libwebsockets >= 3.x, we use this event to mark connections as writable in the event loop */
		case LWS_CALLBACK_EVENT_WAIT_CANCELLED: {
			if (event_callback)
				event_callback->onWaitCancelledCallback(ws_conn);			
			return 0;
		}
#endif		
		case LWS_CALLBACK_CLIENT_WRITEABLE:
		case LWS_CALLBACK_SERVER_WRITEABLE: {
			if(ws_conn == NULL || ws_conn->wsi == NULL) {
				RTC_LOG(LS_VERBOSE) << log_prefix << " Invalid WebSocket client instance...";
				return -1;
			}
			if (event_callback)
				event_callback->onWriteableCallback(ws_conn);
			return 0;
		}
		
		case LWS_CALLBACK_CLIENT_CLOSED:
		case LWS_CALLBACK_CLOSED: {
			if (event_callback)
				event_callback->onDestroyCallback(ws_conn);			
			RTC_LOG(LS_VERBOSE) << log_prefix << "-- closed";
			return 0;
		}
		
		case LWS_CALLBACK_WSI_DESTROY: {
			if (event_callback)
				event_callback->onDestroyCallback(ws_conn);	
			RTC_LOG(LS_VERBOSE) << log_prefix << "-- destroyed";
			return 0;
		}
			
		default: 
			break;
	}

	return 0;
}


/* This callback handles Gateway API requests */
static int websockets_transport_callback(
		struct lws *wsi,
		enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	return websockets_transport_common_callback(wsi, reason, user, in, len, false);
}

static int websockets_transport_callback_secure(
		struct lws *wsi,
		enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	/* We just forward the event to the Gateway API handler */
	return websockets_transport_callback(wsi, reason, user, in, len);
}

/* This callback handles Admin API requests */
static int websockets_transport_admin_callback(
		struct lws *wsi,
		enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	return websockets_transport_common_callback(wsi, reason, user, in, len, true);
}

static int websockets_transport_admin_callback_secure(
		struct lws *wsi,
		enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	/* We just forward the event to the Admin API handler */
	return websockets_transport_admin_callback(wsi, reason, user, in, len);
}

/* WebSockets */
static int websockets_transport_callback_http(
		struct lws *wsi,
		enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	/* This endpoint cannot be used for HTTP */
	switch(reason) {
		case LWS_CALLBACK_HTTP:
			//JANUS_LOG(LOG_VERB, "Rejecting incoming HTTP request on WebSockets endpoint\n");
			lws_return_http_status(wsi, 403, NULL);
			/* Close and free connection */
			return -1;
		case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
			if (!in) {
				//JANUS_LOG(LOG_VERB, "Rejecting incoming HTTP request on WebSockets endpoint: no sub-protocol specified\n");
				return -1;
			}
			break;
		case LWS_CALLBACK_GET_THREAD_ID:
			return (uint64_t)pthread_self();
		default:
			break;
	}
	return 0;
}

static int websockets_transport_callback_https(
		struct lws *wsi,
		enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	/* We just forward the event to the HTTP handler */
	return websockets_transport_callback_http(wsi, reason, user, in, len);
}


/* Protocol mappings */
static struct lws_protocols ws_protocols[] = {
	{ "http-only", websockets_transport_callback_http, 0, 0 },
	{ "gateway-protocol", websockets_transport_callback, sizeof(websockets_transport_session), 0 },
	{ NULL, NULL, 0 }
};
static struct lws_protocols sws_protocols[] = {
	{ "http-only", websockets_transport_callback_https, 0, 0 },
	{ "gateway-protocol", websockets_transport_callback_secure, sizeof(websockets_transport_session), 0 },
	{ NULL, NULL, 0 }
};
static struct lws_protocols admin_ws_protocols[] = {
	{ "http-only", websockets_transport_callback_http, 0, 0 },
	{ "gateway-admin-protocol", websockets_transport_admin_callback, sizeof(websockets_transport_session), 0 },
	{ NULL, NULL, 0 }
};
static struct lws_protocols admin_sws_protocols[] = {
	{ "http-only", websockets_transport_callback_https, 0, 0 },
	{ "gateway-admin-protocol", websockets_transport_admin_callback_secure, sizeof(websockets_transport_session), 0 },
	{ NULL, NULL, 0 }
};


static void websockets_transport_log_emit_function(int level, const char *line) {
	/* FIXME Do we want to use different Gateway debug levels according to the level here? */
	RTC_LOG(INFO) << "[libwebsockets] "
								<< websockets_transport_get_level_str(level)
								<< line;
}

WebsocketsTransport::WebsocketsTransport(ITransportCallbacks *callback)
	: rtc::Thread(std::unique_ptr<rtc::SocketServer>(new rtc::NullSocketServer())),
		callback_(callback)
{
	//RTC_DCHECK(callback_);
	config_ = NULL;
	memset(&ws_context_,0,sizeof(ws_context));
	memset(&ws_context_.s_context,0,sizeof(ws_server_context));
	memset(&ws_context_.c_context,0,sizeof(ws_client_context));
	atomic_int_set(&initialized, 0);
	atomic_int_set(&stopping, 0);
}

WebsocketsTransport::~WebsocketsTransport()
{
	disconnect();
	callback_ = NULL;
}

int WebsocketsTransport::connect(const std::string url,int port)
{
	if (atomic_int_get(&initialized)) {
		RTC_LOG(LERROR) << "libwebsocket has initialized";
		return -1;
	}
	/* Enable libwebsockets logging */
	ws_context_.ws_log_level = LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO; /*|
		LLL_DEBUG | LLL_PARSER | LLL_HEADER | LLL_EXT |
#if (LWS_LIBRARY_VERSION_MAJOR >= 2 && LWS_LIBRARY_VERSION_MINOR >= 2) || (LWS_LIBRARY_VERSION_MAJOR >= 3)
		LLL_CLIENT | LLL_LATENCY | LLL_USER | LLL_COUNT;
#else
		LLL_CLIENT | LLL_LATENCY | LLL_COUNT;
#endif*/
		
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);
	info.port = CONTEXT_PORT_NO_LISTEN;
	info.iface = NULL;
	info.protocols = sws_protocols;
	info.extensions = NULL;
	info.gid = -1;
	info.uid = -1;	
	info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	
	/* Create the base context */
	ws_context_.wsc = lws_create_context(&info);
	if(ws_context_.wsc == NULL) {
		RTC_LOG(INFO) << "Error creating libwebsockets context...";
		if (config_ != NULL)
			gateway_cfg_destory(config_);
		return -1; /* No point in keeping the plugin loaded */
	}

	static struct lws_client_connect_info client_info; 
	memset(&client_info, 0, sizeof(client_info)); 
	client_info.context = ws_context_.wsc;
	client_info.port = port;
	client_info.address = url.c_str();
	client_info.path = "/";
	client_info.host = url.c_str();
	client_info.origin = url.c_str();
	client_info.ssl_connection |= LCCSCF_USE_SSL;
	client_info.ssl_connection |= LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
	client_info.ssl_connection |= LCCSCF_ALLOW_SELFSIGNED;		
	client_info.protocol = sws_protocols[1].name;	
	
	ws_context_.c_context.wss = 
			lws_client_connect_via_info(&client_info);
	if(ws_context_.c_context.wss == NULL) {
		RTC_LOG(INFO) << "Error creating libwebsockets client...";
	}

	if(!ws_context_.c_context.wss) {
		RTC_LOG(INFO) << "No WebSockets server started, giving up...";
		lws_context_destroy(ws_context_.wsc);
		return -1;	/* No point in keeping the plugin loaded */
	}	
	ws_context_.client = true;
	atomic_int_set(&initialized, 1);
	event_callback = this; 	
	this->SetName("websockets_thread", nullptr);
	if (!this->Start()) {
		atomic_int_set(&initialized, 0);
		RTC_LOG(LERROR) << 
				"start Websockets thread error";
		return -1;
	}
	RTC_LOG(INFO) << "start CONNECT TO SERVER";
	return 0;
}

/**
 * connect to server or listen when using server
 */
int WebsocketsTransport::connect(std::string cfg_path)
{
	if(atomic_int_get(&stopping)) {
		/* Still stopping from before */
		return -1;
	}
	
	if(cfg_path.empty()) {
		/* Invalid arguments */
		RTC_LOG(LERROR) << "Invalid arguments";
		return -1;
	}
		
	if (config_ != NULL) {
		gateway_cfg_destory(config_);
		config_ = NULL;
	}
	cfg_path_ = cfg_path;

	if (atomic_int_get(&initialized)) {
		RTC_LOG(LERROR) << "libwebsocket has initialized";
		return -1;
	}

	
	struct lws_context_creation_info wscinfo;
	memset(&wscinfo, 0, sizeof wscinfo);
	wscinfo.options |= LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
	
	/* Read configuration */
	std::string filename = cfg_path_;
	filename.append("/");
	filename.append(GATEWAY_WEBSOCKETS_PACKAGE);
	filename.append(".cfg");
	RTC_LOG(INFO) << "Configuration file: "
								<< filename;
	config_ = gateway_cfg_parse(filename.c_str());
	if(config_ == NULL) {
		RTC_LOG(WARNING) << "Couldn't find .cfg configuration file "
									<< GATEWAY_WEBSOCKETS_PACKAGE;
		return -1;
	}
	const char* ws_logging;
	if (gateway_cfg_get_string(
		config_,gateway_cfg_type_general,"ws_logging",&ws_logging)) {
		RTC_LOG(INFO) << "ws_logging:" << ws_logging;
		if (!strcasecmp(ws_logging, "none")) {
			/* libwebsockets uses a mask to set log levels, as documented here:
			 * https://libwebsockets.org/lws-api-doc-master/html/group__log.html */
		} else if (!strcasecmp(ws_logging, "all")){
				/* Enable all libwebsockets logging */
				ws_context_.ws_log_level = LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO |
					LLL_DEBUG | LLL_PARSER | LLL_HEADER | LLL_EXT |
#if (LWS_LIBRARY_VERSION_MAJOR >= 2 && LWS_LIBRARY_VERSION_MINOR >= 2) || (LWS_LIBRARY_VERSION_MAJOR >= 3)
					LLL_CLIENT | LLL_LATENCY | LLL_USER | LLL_COUNT;
#else
					LLL_CLIENT | LLL_LATENCY | LLL_COUNT;
#endif			
		} else {
			/* Only enable some of the properties */
			std::string string_ws_logging = ws_logging;
			
			if(string_ws_logging.find("err") != std::string::npos)
				ws_context_.ws_log_level |= LLL_ERR;
			if(string_ws_logging.find("warn") != std::string::npos)
				ws_context_.ws_log_level |= LLL_WARN;
			if(string_ws_logging.find("notice") != std::string::npos)
				ws_context_.ws_log_level |= LLL_NOTICE;
			if(string_ws_logging.find("info") != std::string::npos)
				ws_context_.ws_log_level |= LLL_INFO;
			if(string_ws_logging.find("debug") != std::string::npos)
				ws_context_.ws_log_level |= LLL_DEBUG;
			if(string_ws_logging.find("parser") != std::string::npos)
				ws_context_.ws_log_level |= LLL_PARSER;
			if(string_ws_logging.find("header") != std::string::npos)
				ws_context_.ws_log_level |= LLL_HEADER;
			if(string_ws_logging.find("ext") != std::string::npos)
				ws_context_.ws_log_level |= LLL_EXT;
			if(string_ws_logging.find("client") != std::string::npos)
				ws_context_.ws_log_level |= LLL_CLIENT;
			if(string_ws_logging.find("latency") != std::string::npos)
				ws_context_.ws_log_level |= LLL_LATENCY;
#if (LWS_LIBRARY_VERSION_MAJOR >= 2 && LWS_LIBRARY_VERSION_MINOR >= 2) || (LWS_LIBRARY_VERSION_MAJOR >= 3)
			if(string_ws_logging.find("user") != std::string::npos)
				ws_context_.ws_log_level |= LLL_USER;
#endif
			if(string_ws_logging.find("count") != std::string::npos)
				ws_context_.ws_log_level |= LLL_COUNT;
		}
		RTC_LOG(LS_VERBOSE) << "libwebsockets logging: " << ws_context_.ws_log_level;
		lws_set_log_level(ws_context_.ws_log_level, websockets_transport_log_emit_function);
	}
	
	const char* ws_acl;
	/* Any ACL for either the Gateway or Admin API? */
	if (gateway_cfg_get_string(
		config_,gateway_cfg_type_general,"ws_acl",&ws_acl)) {
		std::vector<std::string> list = absl::StrSplit(ws_acl, ",");
		for (int i=0;i< list.size();i++) {
			if (!list[i].empty()) {
				rtc::CritScope lock(&access_list_lock_);
				ws_access_list_.push_back(list[i]);
			}
		}
		RTC_LOG(LS_VERBOSE) << "ws_acl:" << ws_acl;
	} 
	
	const char* admin_ws_acl;
	if (gateway_cfg_get_string(
		config_,gateway_cfg_type_admin,"admin_ws_acl",&admin_ws_acl)) {
		std::vector<std::string> list = absl::StrSplit(admin_ws_acl, ",");
		for (int i=0;i< list.size();i++) {
			if (!list[i].empty()) {
				rtc::CritScope lock(&access_list_lock_);	
				ws_admin_access_list_.push_back(list[i]);
			}
		}
		RTC_LOG(LS_VERBOSE) << "admin_ws_acl:" << admin_ws_acl;
	} 
	
	/* Check if we need to enable the transport level ping/pong mechanism */
	int pingpong_trigger = 0, pingpong_timeout = 0;
	if (gateway_cfg_get_int (
		config_,gateway_cfg_type_general,"pingpong_trigger",&pingpong_trigger))	{
#if (LWS_LIBRARY_VERSION_MAJOR >= 2 && LWS_LIBRARY_VERSION_MINOR >= 1) || (LWS_LIBRARY_VERSION_MAJOR >= 3)
		if(pingpong_trigger < 0) {
			RTC_LOG(INFO) << "Invalid value for pingpong_trigger "
										<< pingpong_trigger
										<< ", ignoring...";
			
			pingpong_trigger = 0;
		}
#else
		RTC_LOG(INFO) << "WebSockets ping/pong only supported in libwebsockets >= 2.1";
#endif
		RTC_LOG(LS_VERBOSE) << "pingpong_trigger:" << pingpong_trigger;
	}

	if (gateway_cfg_get_int(
		config_,gateway_cfg_type_general,"pingpong_timeout",&pingpong_timeout))	{
#if (LWS_LIBRARY_VERSION_MAJOR >= 2 && LWS_LIBRARY_VERSION_MINOR >= 1) || (LWS_LIBRARY_VERSION_MAJOR >= 3)
		if(pingpong_trigger < 0) {
			RTC_LOG(INFO) << "Invalid value for pingpong_trigger "
										<< pingpong_trigger
										<< ", ignoring...";
			pingpong_trigger = 0;
		}
#else
		RTC_LOG(INFO) << "WebSockets ping/pong only supported in libwebsockets >= 2.1";
#endif
		RTC_LOG(LS_VERBOSE) << "pingpong_timeout:" << pingpong_timeout;
	}

	if((pingpong_trigger && !pingpong_timeout) 
			|| (!pingpong_trigger && pingpong_timeout)) {
		RTC_LOG(INFO) << "pingpong_trigger and pingpong_timeout not both set, ignoring...";	
	}
	
#if (LWS_LIBRARY_VERSION_MAJOR >= 2 && LWS_LIBRARY_VERSION_MINOR >= 1) || (LWS_LIBRARY_VERSION_MAJOR >= 3)
	if(pingpong_trigger > 0 && pingpong_timeout > 0) {
		wscinfo.ws_ping_pong_interval = pingpong_trigger;
		wscinfo.timeout_secs = pingpong_timeout;
	}
#endif
	/* Force single-thread server */
	wscinfo.count_threads = 1;	
	/* Create the base context */
	ws_context_.wsc = lws_create_context(&wscinfo);
	if(ws_context_.wsc == NULL) {
		RTC_LOG(INFO) << "Error creating libwebsockets context...";
		gateway_cfg_destory(config_);
		return -1;//o point in keeping the plugin loaded */
	}
	
	/* Setup the Gateway API WebSockets server(s) */
	bool ws_enable = false;
	if (!gateway_cfg_get_bool(config_,gateway_cfg_type_general,"ws",&ws_enable)) {
		RTC_LOG(INFO) << "WebSockets server disabled";
	} else {
		int ws_port = 8188;
		const char *ws_interface;
		const char *ws_ip;
	
		if (!gateway_cfg_get_int(
				config_,gateway_cfg_type_general,"ws_port",&ws_port))	{
			RTC_LOG(INFO) << "Invalid port "	
										<< ws_port
										<< ", falling back to default";	
			ws_port = 8188;
		}
		
		if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_general,"ws_interface",&ws_interface)) {
			ws_interface = NULL;
		}
			
		if (gateway_cfg_get_string(
			config_,gateway_cfg_type_general,"ws_ip",&ws_ip)) {
			char *iface = websockets_transport_get_interface_name(ws_ip);
			if(iface == NULL) {
				RTC_LOG(INFO) << "No interface associated with "	
											<< ws_ip
											<< "?  Falling back to no interface...";					
			}
			ws_ip = iface;			
		}	else {
			ws_ip = NULL;
		}
		/* Prepare context */
		struct lws_context_creation_info info;
		memset(&info, 0, sizeof info);
		info.port = ws_port;
		info.iface = ws_ip ? ws_ip : ws_interface;
		info.protocols = ws_protocols;
		info.extensions = NULL;
		info.ssl_cert_filepath = NULL;
		info.ssl_private_key_filepath = NULL;
		info.ssl_private_key_password = NULL;
		info.gid = -1;
		info.uid = -1;
		info.options = 0;
		/* Create the WebSocket context */
		ws_context_.s_context.wss = 
			lws_create_vhost(ws_context_.wsc, &info);
		if(ws_context_.s_context.wss == NULL) {
			RTC_LOG(INFO) << "Error creating vhost for WebSockets server..."; 			
		} else {
			RTC_LOG(INFO) << "WebSockets server started (port " << ws_port << ")...";
		}
	}
	
	bool wss_enable = false;
	if (!gateway_cfg_get_bool(config_,gateway_cfg_type_general,"wss",&wss_enable)) {
		RTC_LOG(INFO) << "Secure WebSockets server disabled";
	} else {
		int wss_port = 8989;
		const char *wss_interface;
		const char *wss_ip;
		const char *cert_pem;
		const char *cert_key;		
		const char *cert_pwd;		
		if (!gateway_cfg_get_int(config_,gateway_cfg_type_general,"wss_port",&wss_port))	{
			RTC_LOG(INFO) << "Invalid port "	
										<< wss_port
										<< ", falling back to default";	
			wss_port = 8989;
		}
		
		if (!gateway_cfg_get_string(
				config_,gateway_cfg_type_general,"wss_interface",&wss_interface)) {
			wss_interface = NULL;
		}
			
		if (gateway_cfg_get_string(
			config_,gateway_cfg_type_general,"wss_ip",&wss_ip)) {
			char *iface = websockets_transport_get_interface_name(wss_ip);
			if(iface == NULL) {
				RTC_LOG(INFO) << "No interface associated with "	
											<< wss_ip
											<< "?  Falling back to no interface...";					
			}
			RTC_LOG(LS_VERBOSE) << "iface: " << iface;
			wss_ip = iface;			
		}	else {
			wss_ip = NULL;
		}
		
		if (!gateway_cfg_get_string(config_,
						gateway_cfg_type_certificates,"cert_pem",&cert_pem) || 
				!gateway_cfg_get_string(config_,
						gateway_cfg_type_certificates,"cert_key",&cert_key)) {
				RTC_LOG(INFO) << "Missing certificate/key path ";						
		} else {
				if (!gateway_cfg_get_string(config_,
					gateway_cfg_type_certificates,"cert_pwd",&cert_pwd)) {
					cert_pwd = NULL;
				}
				/* Prepare secure context */
				struct lws_context_creation_info info;
				memset(&info, 0, sizeof info);
				info.port = wss_port;
				info.iface = wss_ip ? wss_ip : wss_interface;
				info.protocols = sws_protocols;
				info.extensions = NULL;
				info.ssl_cert_filepath = cert_pem;
				info.ssl_private_key_filepath = cert_key;
				info.ssl_private_key_password = cert_pwd;
				info.gid = -1;
				info.uid = -1;
				info.options = 0;
#if LWS_LIBRARY_VERSION_MAJOR >= 2
				info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#else
				info.options = 0;
#endif
				/* Create the secure WebSocket context */
				ws_context_.s_context.swss = 
					lws_create_vhost(ws_context_.wsc, &info);
				if(ws_context_.s_context.swss == NULL) {
					RTC_LOG(INFO) << "Error creating vhost for Secure WebSockets server...";	
				} else {
					RTC_LOG(INFO) << "secure WebSockets server started (port " << wss_port<< ")...";
				}			
		}
	}
	
	/* Do the same for the Admin API, if enabled */
	bool admin_ws = false;
	if (!gateway_cfg_get_bool(config_,gateway_cfg_type_admin,"admin_ws",&admin_ws)) {
		RTC_LOG(INFO) << "WebSockets server disabled";
	} else {
		int admin_ws_port = 7188;
		const char *admin_ws_interface;
		const char *admin_ws_ip;	
		if (!gateway_cfg_get_int(
				config_,gateway_cfg_type_admin,"admin_ws_port",&admin_ws_port)) {
			RTC_LOG(INFO) << "Invalid admin port "	
										<< admin_ws_port
										<< ", falling back to default"; 
			admin_ws_port = 7188;
		}
		
		if (!gateway_cfg_get_string(
			config_,gateway_cfg_type_admin,"admin_ws_interface",&admin_ws_interface)) {
			admin_ws_interface = NULL;
		}
			
		if (gateway_cfg_get_string(
			config_,gateway_cfg_type_admin,"admin_ws_ip",&admin_ws_ip)) {
			char *iface = websockets_transport_get_interface_name(admin_ws_ip);
			if(iface == NULL) {
				RTC_LOG(INFO) << "No interface associated with "	
											<< admin_ws_ip
											<< "?  Falling back to no interface...";					
			}
			admin_ws_ip = iface;			
		} else {
			admin_ws_ip = NULL;
		}
		
		/* Prepare context */
		struct lws_context_creation_info info;
		memset(&info, 0, sizeof info);
		info.port = admin_ws_port;
		info.iface = admin_ws_ip ? admin_ws_ip : admin_ws_interface;
		info.protocols = admin_ws_protocols;
		info.extensions = NULL;
		info.ssl_cert_filepath = NULL;
		info.ssl_private_key_filepath = NULL;
		info.ssl_private_key_password = NULL;
		info.gid = -1;
		info.uid = -1;
		info.options = 0;	
		/* Create the WebSocket context */
		ws_context_.s_context.admin_wss = lws_create_vhost(ws_context_.wsc, &info);
		if(ws_context_.s_context.admin_wss == NULL) {
			RTC_LOG(INFO) << "Error creating vhost for admin WebSockets server..."; 			
		} else {
			RTC_LOG(INFO) << "WebSockets server started (port " << admin_ws_port << ")...";
		}
	}

	bool admin_wss = false;
	if (!gateway_cfg_get_bool(config_,gateway_cfg_type_admin,"admin_wss",&admin_wss)) {
		RTC_LOG(INFO) << "Secure WebSockets server disabled";
	} else {
		int admin_wss_port = 7189;
		const char *admin_wss_interface;
		const char *admin_wss_ip;
		const char *cert_pem;
		const char *cert_key;		
		const char *cert_pwd;		
		if (!gateway_cfg_get_int(config_,gateway_cfg_type_admin,"admin_wss_port",&admin_wss_port))	{
			RTC_LOG(INFO) << "Invalid port "	
										<< admin_wss_port
										<< ", falling back to default"; 
			admin_wss_port = 7189;
		}
		
		if (!gateway_cfg_get_string(
				config_,gateway_cfg_type_admin,"admin_wss_interface",&admin_wss_interface)) {
			admin_wss_interface = NULL;
		}
			
		if (gateway_cfg_get_string(
			config_,gateway_cfg_type_admin,"admin_wss_ip",&admin_wss_ip)) {
			char *iface = websockets_transport_get_interface_name(admin_wss_ip);
			if(iface == NULL) {
				RTC_LOG(INFO) << "No interface associated with "	
											<< admin_wss_ip
											<< "?  Falling back to no interface...";					
			}
			admin_wss_ip = iface; 		
		} else {
			admin_wss_ip = NULL;
		}
		
		if (!gateway_cfg_get_string(config_,gateway_cfg_type_certificates,"cert_pem",&cert_pem) || 
				!gateway_cfg_get_string(config_,gateway_cfg_type_certificates,"cert_key",&cert_key)){
				RTC_LOG(INFO) << "@@@Missing certificate/key path @@@@"; 					
		} else {
				if (gateway_cfg_get_string(config_,
					gateway_cfg_type_certificates,"cert_pwd",&cert_pwd)) {
					cert_pwd = NULL;
				}	
				/* Prepare secure context */
				struct lws_context_creation_info info;
				memset(&info, 0, sizeof info);
				info.port = admin_wss_port;
				info.iface = admin_wss_ip ? admin_wss_ip : admin_wss_interface;
				info.protocols = admin_sws_protocols;
				info.extensions = NULL;
				info.ssl_cert_filepath = cert_pem;
				info.ssl_private_key_filepath = cert_key;
				info.ssl_private_key_password = cert_pwd;
				info.gid = -1;
				info.uid = -1;
#if LWS_LIBRARY_VERSION_MAJOR >= 2
				info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#else
				info.options = 0;
#endif
				/* Create the secure WebSocket context */
				ws_context_.s_context.admin_swss = 
					lws_create_vhost(ws_context_.wsc, &info);
				if(ws_context_.s_context.admin_swss == NULL) {
					RTC_LOG(INFO) << "Error creating vhost for admin Secure WebSockets server...";	
				} else {
					RTC_LOG(INFO) << "Secure WebSockets server started (port " << admin_wss_port<< ")...";
				} 		
		}
	}

	gateway_cfg_destory(config_);
	config_ = NULL;
	if(!ws_context_.s_context.wss 
			&& !ws_context_.s_context.swss 
			&& !ws_context_.s_context.admin_wss 
			&& !ws_context_.s_context.admin_swss) {
		RTC_LOG(INFO) << "No WebSockets server started, giving up...";
		lws_context_destroy(ws_context_.wsc);
		return -1;	/* No point in keeping the plugin loaded */
	}	
	ws_context_.client = false;
	atomic_int_set(&initialized, 1);
	event_callback = this; 
	this->SetName("websockets_thread", nullptr);
	if (!this->Start()) {
		atomic_int_set(&initialized, 0);
		RTC_LOG(LERROR) << 
				"start Websockets thread error";
		return -1;
	}
	return 0;
}

/**
 * disconnect the server or client
 */
int WebsocketsTransport::disconnect()
{
	if(!atomic_int_get(&initialized))
		return 0;
	atomic_int_set(&stopping, 1);
	/* Stop the service thread */
	this->Join();
	
	/* Destroy the context */
	if(ws_context_.wsc != NULL) {
		lws_context_destroy(ws_context_.wsc);
		ws_context_.wsc = NULL;
	}
#if (LWS_LIBRARY_VERSION_MAJOR >= 3)
	{
		rtc::CritScope lock(&sessionsLock);
		if (!sessions_.empty())
			sessions_.clear();
		if (!writable_sessions_.empty())
			writable_sessions_.clear();
	}
#endif
	atomic_int_set(&initialized, 0);
	atomic_int_set(&stopping, 0);
	return 0;
}

/**
 * disconnect
 */
void WebsocketsTransport::destory()
{
	delete this;
}

/**
 * @param session
 * @param message
 */
int WebsocketsTransport::sendMessage(
	gateway_transport_session session,const std::string& message)
{
	if(message.empty() 
		|| session == nullptr 
		|| atomic_int_get(&session->destroyed))
		return -1;

	gateway_mutex_lock(&session->mutex);
	websockets_transport_session *conn = 
			(websockets_transport_session *)session->transport_p;
	
	if(!conn || !conn->wsi || atomic_int_get(&conn->destroyed)) {
		gateway_mutex_unlock(&session->mutex);
		return -1;
	}
	
	async_queue_element* element = transport_message_new(&conn->messages); 
	if (!element->data){
		gateway_mutex_unlock(&session->mutex);
		return -1;
	}
	transport_message* payload = 
			(transport_message*)element->data;
	payload->msg = strdup(message.c_str());
	transport_message_push(&conn->messages, element); 
#if (LWS_LIBRARY_VERSION_MAJOR >= 3)
	/* On libwebsockets >= 3.x we use lws_cancel_service */
	{
		rtc::CritScope lock(&sessionsLock);
		auto iterator = writable_sessions_.find(conn);
		if (iterator == writable_sessions_.end() 
			&& !writable_sessions_.count(conn)) {
				RTC_LOG(INFO) << "insert conn to writable_sessions_";
				writable_sessions_.insert(
					std::pair<websockets_transport_session*,websockets_transport_session*>
					(conn,conn/*iterator->second,iterator->second*/)
				);				
		} 	
		lws_cancel_service(ws_context_.wsc);
	}
#else
	/* On libwebsockets < 3.x we use lws_callback_on_writable */
	{
		rtc::CritScope lock(&sessionsLock);
		lws_callback_on_writable(conn->wsi);
	}
#endif
	gateway_mutex_unlock(&session->mutex);
		RTC_LOG(INFO) << __FUNCTION__ << " success";
	return 0;
}


/**
 * @param session
 * @param request_id
 * @param admin
 * @param message
 */
int WebsocketsTransport::sendMessage(
		gateway_transport_session session,
  	void *request_id, bool admin,const std::string& message)
{
	return sendMessage(session,message);
}

/**
 * return the transport type
 */
TransportType WebsocketsTransport::getType()
{
	return TRANSPORT_WEBSOCKET;
}
/**
 * @return transport module package
 */
std::string WebsocketsTransport::getPackage()
{
	return GATEWAY_WEBSOCKETS_PACKAGE;
}
/**
 * @return
 */
bool WebsocketsTransport::checkAllowed(const std::string& ipaddress, bool admin)
{
	if(ipaddress.empty())
		return false;
	
	if (ws_context_.client)
		return true;
	
	rtc::CritScope lock(&access_list_lock_);
	if(!admin && ws_access_list_.empty()) {
		RTC_LOG(LS_VERBOSE) << "Yep";
		return true;
	}
	
	if(admin && ws_admin_access_list_.empty()) {
		RTC_LOG(LS_VERBOSE) << "Yep";
		return true;
	}	
	
	std::vector<std::string>* access_list = 
			admin ? &ws_admin_access_list_ : &ws_access_list_;
	for (auto it = access_list->begin();
		it != access_list->end(); it ++) {
		if (ipaddress.find(*it) != std::string::npos)
			return true;
	}	
	return false;
}

/**
 * @param client
 */ 
void WebsocketsTransport::onConnectedCallback(websockets_transport_session* session)
{
	if (!session || !session->wsi)
		return;

	const char *log_prefix = session->admin ? "AdminWSS" : "WSS";	
	/* Is there any filtering we should apply? */
	char ip[256];
#ifdef HAVE_LIBWEBSOCKETS_PEER_SIMPLE
	lws_get_peer_simple(session->wsi, ip, 256);
	RTC_LOG(LS_VERBOSE) << log_prefix << " WebSocket connection opened from " << ip;
#else
	char name[256];
	lws_get_peer_addresses(session->wsi, lws_get_socket_fd(session->wsi), name, 256, ip, 256);
	RTC_LOG(LS_VERBOSE) << log_prefix << " WebSocket connection opened from " << ip << " by " << name;
#endif
	if(!checkAllowed(ip, session->admin)) {
		RTC_LOG(LERROR) << log_prefix << "IP " << ip 
										<< " is unauthorized to connect to the WebSockets "
										<< (session->admin ? "Admin" : "gateway")
										<< " API interface";
		/* Close the connection */
		lws_callback_on_writable(session->wsi);
		return;
	}
	transport_message_queue_init(&session->messages,32);
	session->ts = gateway_transport_session_create(session,NULL);
	
#if (LWS_LIBRARY_VERSION_MAJOR >= 3)
	{
		rtc::CritScope lock(&sessionsLock);
		sessions_.insert(
			std::pair<websockets_transport_session*,websockets_transport_session*>
			(session,session)
		);		
	}
#endif
	/* Let us know when the WebSocket channel becomes writeable */
	lws_callback_on_writable(session->wsi);	
	if (callback_ != NULL) {
		Json::Value obj;
		obj["event"] = "connected";
		obj["admin_api"] = session->admin ? true : false;
		obj["ip"] = ip;
		Json::StyledWriter writer;
		callback_->NotifyEvent(this,session->ts,writer.write(std::move(obj)));
	}
}

/**
 * @param session
 */ 	
void WebsocketsTransport::onReceivedCallback(websockets_transport_session* session) 
{
	if (!session || !session->wsi)
		return;
	if (callback_ != NULL && 
			session->incoming != NULL)
		callback_->IncomingRequest(
			this,
			session->ts,
			NULL,
			session->admin,
			strdup(session->incoming));
}

#if (LWS_LIBRARY_VERSION_MAJOR >= 3)	
void WebsocketsTransport::onWaitCancelledCallback(websockets_transport_session* session)
{
	rtc::CritScope lock(&sessionsLock);
	if (!writable_sessions_.empty()) {
		for(auto iterator=writable_sessions_.begin();
			iterator != writable_sessions_.end(); iterator++) {
			websockets_transport_session* session_ = 
					(websockets_transport_session*)iterator->second;
			if (session_ == NULL || session_->wsi == NULL)
				continue;
			lws_callback_on_writable(session_->wsi);
		}
		writable_sessions_.clear();
	}
		
}
#endif

void WebsocketsTransport::onWriteableCallback(websockets_transport_session* session)
{

	if (!session || !session->wsi)
		return;
	
	if (atomic_int_get(&session->destroyed) 
		|| atomic_int_get(&stopping)) return;
	
	gateway_mutex_lock(&session->ts->mutex);
	/* Check if we have a pending/partial write to complete first */
	if(session->buffer && session->bufpending > 0 && session->bufoffset > 0
			&& !atomic_int_get(&session->destroyed) && !atomic_int_get(&stopping)) {
		RTC_LOG(LS_VERBOSE) << "Completing pending WebSocket write (still need to write last";
		int sent = lws_write(session->wsi, 
								session->buffer + session->bufoffset, 
								session->bufpending, LWS_WRITE_TEXT);
		RTC_LOG(LS_VERBOSE) << "Sent" << sent << " bytes";
		if(sent > -1 && sent < session->bufpending) {
			/* We still couldn't send everything that was left, we'll try and complete this in the next round */
			session->bufpending -= sent;
			session->bufoffset += sent;
		} else {
			/* Clear the pending/partial write queue */
			session->bufpending = 0;
			session->bufoffset = 0;
		}
		/* Done for this round, check the next response/notification later */
		lws_callback_on_writable(session->wsi);
		gateway_mutex_unlock(&session->ts->mutex);
		return;
	}
	
	/* Shoot all the pending messages */
	async_queue_element* element = transport_message_pop(&session->messages);
	if (!element || !element->data) {
		gateway_mutex_unlock(&session->ts->mutex);
		return;
	}
	
	transport_message* sendMessage = (transport_message*)element->data;
	if (sendMessage == NULL || sendMessage->msg == NULL) {
		gateway_mutex_unlock(&session->ts->mutex);
		return;
	}
	
	char *response = sendMessage->msg;
	
	if(response && !atomic_int_get(&session->destroyed) && !atomic_int_get(&stopping)) {
		/* Gotcha! */
		int buflen = 
			LWS_SEND_BUFFER_PRE_PADDING + strlen(response) + LWS_SEND_BUFFER_POST_PADDING;
		if (buflen > session->buflen) {
			/* We need a larger shared buffer */
			session->buflen = buflen;
			session->buffer = (unsigned char *)realloc(session->buffer, buflen);
		}
		memcpy(session->buffer + LWS_SEND_BUFFER_PRE_PADDING, 
			response, strlen(response));
		int sent = lws_write(session->wsi, 
					session->buffer + LWS_SEND_BUFFER_PRE_PADDING, 
					strlen(response), 
					LWS_WRITE_TEXT);
		
		if(sent > -1 && sent < (int)strlen(response)) {
			/* We couldn't send everything in a single write, we'll complete this in the next round */
			session->bufpending = strlen(response) - sent;
			session->bufoffset = LWS_SEND_BUFFER_PRE_PADDING + sent;
		}
		/* We can get rid of the message */
		transport_message_free(&session->messages,element);
		/* Done for this round, check the next response/notification later */
		lws_callback_on_writable(session->wsi);
		gateway_mutex_unlock(&session->ts->mutex);
		return ;
	}
	gateway_mutex_unlock(&session->ts->mutex);


}
/**
 * @param client
 */ 
void WebsocketsTransport::onDestroyCallback(websockets_transport_session* session)
{
	
	if(!session || !session->ts)
		return;
	
	gateway_mutex_lock(&session->ts->mutex);
	if(!atomic_int_compare_and_exchange(&session->destroyed, 0, 1)) {
		gateway_mutex_unlock(&session->ts->mutex);
		return;
	}
	/* Cleanup */
	RTC_LOG(INFO) << "Destroying WebSocket conn";
	
#if (LWS_LIBRARY_VERSION_MAJOR >= 3)
	{
		rtc::CritScope lock(&sessionsLock);
		if (sessions_.count(session))
			sessions_.erase(session);	
		if (writable_sessions_.count(session))
			writable_sessions_.erase(session);			
	}
#endif
	session->wsi = NULL;
	/* Notify handlers about this transport being gone */
	if (callback_ != NULL) {
		Json::Value obj;
		obj["event"] = "disconnected";
		Json::StyledWriter writer;
		callback_->NotifyEvent(this,session->ts,writer.write(obj));
	}	
	session->ts->transport_p = NULL;
	/* Remove messages queue too, if needed */
	transport_message_queue_clean(&session->messages);
	/* ... and the shared buffers */
	if (session->incoming != NULL)
		free(session->incoming);
	session->incoming = NULL;
	if (session->buffer != NULL)
		free(session->buffer);
	session->buffer = NULL;
	session->buflen = 0;
	session->bufpending = 0;
	session->bufoffset = 0;
	gateway_mutex_unlock(&session->ts->mutex);
	if (callback_ != NULL)
		callback_->TransportGone(this,session->ts);
	/* Notify core */
	gateway_transport_session_destroy(session->ts);
}

void WebsocketsTransport::Run() 
{
	if (ws_context_.wsc == NULL) {
		RTC_LOG(LERROR) << "Invalid service";
		return;
	}
	
	while(atomic_int_get(&initialized) && !atomic_int_get(&stopping)) {
		/* libwebsockets is single thread, we cycle through events here */
		lws_service(ws_context_.wsc, 500);
	}
	/* Get rid of the WebSockets server */
	lws_cancel_service(ws_context_.wsc);
	/* Done */
	RTC_LOG(INFO) << "WebSockets thread ended";
}

