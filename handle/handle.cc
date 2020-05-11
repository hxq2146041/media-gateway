
#include "handle.h"
#include "nice_handle.h"
#include "p2p_handle.h"
#include "gateway.h"

/* RTP/RTCP port range */
uint16_t Handle::rtp_range_min = 0;
/* RTP/RTCP port range */
uint16_t Handle::rtp_range_max = 0;	
/* ICE-Lite status */
bool Handle::nice_lite_enabled;
/* ICE-TCP support (only libnice >= 0.1.8, currently broken) */
bool Handle::nice_tcp_enabled;
/* Full-trickle support */
bool Handle::nice_full_trickle_enabled;
/* mDNS resolution support */
bool Handle::nice_mdns_enabled;
/* IPv6 support (still mostly WIP) */
bool Handle::nice_ipv6_enabled;	

/* STUN server/port, if any */
std::string Handle::nice_stun_server;
uint16_t Handle::nice_stun_port = 0; 
std::string Handle::public_address;


/* TURN server/port and credentials, if any */
std::string Handle::nice_turn_server;
uint16_t Handle::nice_turn_port = 0;
std::string Handle::nice_turn_user;
std::string Handle::nice_turn_pwd;
NiceRelayType Handle::nice_turn_type = NICE_RELAY_TYPE_TURN_UDP; 

bool Handle::nice_debugging_enabled;

std::map<gateway_plugin_session,
	gateway_plugin_session> Handle::plugin_sessions;
rtc::CriticalSection Handle::plugin_sessions_mutex;

/* libnice initialization */
void Handle::init(
	bool ice_lite, bool ice_tcp, bool full_trickle, bool ignore_mdns,
	bool ipv6, uint16_t rtp_min_port, uint16_t rtp_max_port) 
{
	nice_lite_enabled = ice_lite;
	nice_tcp_enabled = ice_tcp;
	nice_full_trickle_enabled = full_trickle;
	nice_mdns_enabled = !ignore_mdns;
	nice_ipv6_enabled = ipv6;
	
	rtc::StringBuilder stringBuilder;
	stringBuilder.AppendFormat(
		"Initializing ICE stuff (%s mode, ICE-TCP candidates %s, %s-trickle, IPv6 support %s)\n",
		nice_lite_enabled ? "Lite" : "Full",
		nice_tcp_enabled ? "enabled" : "disabled",
		nice_full_trickle_enabled ? "full" : "half",
		nice_ipv6_enabled ? "enabled" : "disabled");
	
	RTC_LOG(INFO) << stringBuilder.str();
 	stringBuilder.Clear();
	
	if(nice_tcp_enabled) {
#ifndef HAVE_LIBNICE_TCP
		RTC_LOG(WARNING) << "libnice version < 0.1.8, disabling ICE-TCP support";
		nice_tcp_enabled = FALSE;
#else
		if(!nice_lite_enabled) {
			RTC_LOG(WARNING) << "You may experience problems when having ICE-TCP enabled without having ICE Lite enabled too in libnice";
		}
#endif
	}
	/* libnice debugging is disabled unless explicitly stated */
	nice_debug_disable(TRUE);

	/*! \note The RTP/RTCP port range configuration may be just a placeholder: for
	 * instance, libnice supports this since 0.1.0, but the 0.1.3 on Fedora fails
	 * when linking with an undefined reference to \c nice_agent_set_port_range
	 * so this is checked by the install.sh script in advance. */
	rtp_range_min = rtp_min_port;
	rtp_range_max = rtp_max_port;
	
	if(rtp_range_max < rtp_range_min) {
		stringBuilder.AppendFormat("Invalid ICE port range: %d > %d",rtp_range_min, rtp_range_max);
		RTC_LOG(WARNING) << stringBuilder.str();
		stringBuilder.Clear();
	} else if(rtp_range_min > 0 || rtp_range_max > 0) {
	
#ifndef HAVE_PORTRANGE
		RTC_LOG(WARNING) << "nice_agent_set_port_range unavailable, port range disabled";
#else
		stringBuilder.AppendFormat("ICE port range: %d-%d",rtp_range_min, rtp_range_max);
		RTC_LOG(WARNING) << stringBuilder.str();
		stringBuilder.Clear();

		RTC_LOG(WARNING) << "@@@@@@@@@@@@@@@@@@@@@@";
#endif
	}
	
	if(!nice_mdns_enabled)
		RTC_LOG(WARNING) << "mDNS resolution disabled, .local candidates will be ignored";
	/* We keep track of plugin sessions to avoid problems */
	//plugin_sessions = g_hash_table_new_full(NULL, NULL, NULL, (GDestroyNotify)janus_plugin_session_dereference);
	//janus_mutex_init(&plugin_sessions_mutex);

#ifdef HAVE_TURNRESTAPI
	/* Initialize the TURN REST API client stack, whether we're going to use it or not */
	//janus_turnrest_init();
#endif
}

void Handle::deInit() 
{
#ifdef HAVE_TURNRESTAPI
	//janus_turnrest_deinit();
#endif
}

int Handle::testStunServer(
	network_address& addr, uint16_t port,uint16_t local_port, 
	network_address& public_addr, uint16_t *public_port) 
{

	rtc::StringBuilder stringBuilder;
	/* Test the STUN server */
	StunAgent stun;

	stun_agent_init(&stun, STUN_ALL_KNOWN_ATTRIBUTES, STUN_COMPATIBILITY_RFC5389, StunAgentUsageFlags(0));
	
	StunMessage msg;
	uint8_t buf[1500];

	size_t len = stun_usage_bind_create(&stun, &msg, buf, 1500);
	stringBuilder.AppendFormat("Testing STUN server: message is of %zu bytes\n",len);
	
	RTC_LOG(INFO) << stringBuilder.str();
	stringBuilder.Clear();
	
	/* Use the network_address info to drive the socket creation */
	int fd = socket(addr.family(), SOCK_DGRAM, 0);
	if(fd < 0) {
		RTC_LOG(INFO) << "Error creating socket for STUN BINDING test";
		return -1;
	}
	
	struct sockaddr *address = NULL, *remote = NULL;
	struct sockaddr_in address4 = { 0 }, remote4 = { 0 };
	struct sockaddr_in6 address6 = { 0 }, remote6 = { 0 };
	socklen_t addrlen = 0;
	if(addr.family() == AF_INET) {
		memset(&address4, 0, sizeof(address4));
		address4.sin_family = AF_INET;
		address4.sin_port = htons(local_port);
		address4.sin_addr.s_addr = INADDR_ANY;
		memset(&remote4, 0, sizeof(remote4));
		remote4.sin_family = AF_INET;
		remote4.sin_port = htons(port);
		struct in_addr ipv4 = addr.ipv4_address(); 
		memcpy(&remote4.sin_addr, &ipv4, sizeof(struct in_addr));
		address = (struct sockaddr *)(&address4);
		remote = (struct sockaddr *)(&remote4);
		addrlen = sizeof(remote4);
	} else if(addr.family() == AF_INET6) {
		memset(&address6, 0, sizeof(address6));
		address6.sin6_family = AF_INET6;
		address6.sin6_port = htons(local_port);
		address6.sin6_addr = in6addr_any;
		memset(&remote6, 0, sizeof(remote6));
		remote6.sin6_family = AF_INET6;
		remote6.sin6_port = htons(port);
		struct in6_addr ipv6 = addr.ipv6_address(); 
		memcpy(&remote6.sin6_addr, &ipv6, sizeof(struct in6_addr));
		remote6.sin6_addr = ipv6;
		address = (struct sockaddr *)(&address6);
		remote = (struct sockaddr *)(&remote6);
		addrlen = sizeof(remote6);
	}
	if(bind(fd, address, addrlen) < 0) {
		stringBuilder.AppendFormat("Bind failed for STUN BINDING test: %d (%s)\n",strerror(errno));
		RTC_LOG(LERROR) << stringBuilder.str();
		stringBuilder.Clear();
		close(fd);
		return -1;
	}
	
	int bytes = sendto(fd, buf, len, 0, remote, addrlen);
	if(bytes < 0) {
		RTC_LOG(LERROR) << "Error sending STUN BINDING test";
		close(fd);
		return -1;
	}
	stringBuilder.AppendFormat("  >> Sent %d bytes, waiting for reply...\n",bytes);
	RTC_LOG(LS_VERBOSE) << stringBuilder.str();
	stringBuilder.Clear();
	struct timeval timeout;
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	timeout.tv_sec = 5;	/* FIXME Don't wait forever */
	timeout.tv_usec = 0;
	int err = select(fd+1, &readfds, NULL, NULL, &timeout);
	
	if(err < 0) {
		stringBuilder.AppendFormat( "Error waiting for a response to our STUN BINDING test: %d (%s)\n",errno, strerror(errno));
		RTC_LOG(LERROR) << stringBuilder.str();
		stringBuilder.Clear();		
		close(fd);
		return -1;
	}
	if(!FD_ISSET(fd, &readfds)) {
		RTC_LOG(LERROR)<< "No response to our STUN BINDING test";
		close(fd);
		return -1;
	}
	bytes = recvfrom(fd, buf, 1500, 0, remote, &addrlen);
	stringBuilder.AppendFormat("   >> Got %d bytes...\n",bytes);
	RTC_LOG(LS_VERBOSE) << stringBuilder.str();
	stringBuilder.Clear();	
	close(fd);
	if(bytes < 0) {
		RTC_LOG(LERROR)<< "Failed to receive STUN";
		return -1;
	}
	

	if(stun_agent_validate (&stun, &msg, buf, bytes, NULL, NULL) != STUN_VALIDATION_SUCCESS) {
		RTC_LOG(LERROR)<< "Failed to validate STUN BINDING response";
		return -1;
	}

	StunClass class_ = stun_message_get_class(&msg);
	StunMethod method = stun_message_get_method(&msg);
	
	if(class_ != STUN_RESPONSE || method != STUN_BINDING) {
		RTC_LOG(LERROR)<< "Unexpected STUN response: "
										<< class_ << " " << method;	
		return -1;
	}
	
	StunMessageReturn ret = 
		stun_message_find_xor_addr(&msg, 
			STUN_ATTRIBUTE_XOR_MAPPED_ADDRESS, (struct sockaddr_storage *)address, &addrlen);
	RTC_LOG(LS_VERBOSE) << "  >> XOR-MAPPED-ADDRESS: " << ret;
	
	if(ret == STUN_MESSAGE_RETURN_SUCCESS) {
		if(address->sa_family == AF_INET) {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			network_address temp(addr->sin_addr);
			public_addr = temp;
		} else if(address->sa_family == AF_INET6) {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			network_address temp(addr->sin6_addr);
			public_addr = temp;
		}			
		
		if(public_port != NULL) {
			if(address->sa_family == AF_INET) {
				struct sockaddr_in *addr = (struct sockaddr_in *)address;
				*public_port = ntohs(addr->sin_port);
			} else if(address->sa_family == AF_INET6) {
				struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
				*public_port = ntohs(addr->sin6_port);
			}
		}
		return 0;
	}
	ret = stun_message_find_addr(&msg, STUN_ATTRIBUTE_MAPPED_ADDRESS, 
				(struct sockaddr_storage *)address, &addrlen);
	
	RTC_LOG(LS_VERBOSE) << "  >> MAPPED-ADDRESS: " << ret;
	
	if(ret == STUN_MESSAGE_RETURN_SUCCESS) {
		if(address->sa_family == AF_INET) {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			network_address temp(addr->sin_addr);
			public_addr = std::move(temp);
		} else if(address->sa_family == AF_INET6) {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			network_address temp(addr->sin6_addr);
			public_addr = std::move(temp);
		}				
		if(public_port != NULL) {
			if(address->sa_family == AF_INET) {
				struct sockaddr_in *addr = (struct sockaddr_in *)address;
				*public_port = ntohs(addr->sin_port);
			} else if(address->sa_family == AF_INET6) {
				struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
				*public_port = ntohs(addr->sin6_port);
			}
		}
		return 0;
	}
	
	/* No usable attribute? */
	RTC_LOG(LERROR) << "No XOR-MAPPED-ADDRESS or MAPPED-ADDRESS...";
	return -1;
}

int Handle::setStunServer(
		const std::string& stun_server, uint16_t stun_port) 
{
	if(stun_server.empty())
		return 0;	/* No initialization needed */
	if(stun_port == 0)
		stun_port = 3478;

	rtc::StringBuilder stringBuilder;
	stringBuilder.AppendFormat("STUN server to use: %s:%u\n",stun_server.c_str(), stun_port);
	RTC_LOG(LS_VERBOSE) << stringBuilder.str();
	stringBuilder.Clear();	
	
	/* Resolve address to get an IP */
	struct addrinfo *res = NULL;
	if(getaddrinfo(stun_server.c_str(), NULL, NULL, &res) != 0 ) {
		RTC_LOG(LERROR) << "Could not resolve " << stun_server;
		if(res)
			freeaddrinfo(res);
		return -1;
	}
	struct sockaddr *address = res->ai_addr;
	network_address addr_;
	if (address != NULL) {
		if(address->sa_family == AF_INET) {
			struct sockaddr_in *addr = (struct sockaddr_in *)address;
			network_address temp(addr->sin_addr);
			addr_ =  std::move(temp);
		} else if(address->sa_family == AF_INET6) {
			struct sockaddr_in6 *addr = (struct sockaddr_in6 *)address;
			network_address temp(addr->sin6_addr);
			addr_ =  std::move(temp);
		}	
	}
	
	if (res) {
		freeaddrinfo(res);
		res = NULL;
	}

	if (!nice_stun_server.empty())
		nice_stun_server.clear();
	
	nice_stun_server = addr_.ToString();
	if (nice_stun_server.empty()) {
		RTC_LOG(LERROR) << "Could not resolve %s " << stun_server;
		return -1;		
	}		
	nice_stun_port = stun_port;
	stringBuilder.AppendFormat("  >> %s:%u (%s)\n",nice_stun_server.c_str(),nice_stun_port,addr_.family() == AF_INET ? "IPv4" : "IPv6");
	RTC_LOG(LS_VERBOSE) << stringBuilder.str();
	stringBuilder.Clear();	
	
	/* Test the STUN server */
	network_address public_addr;
	if(testStunServer(addr_, nice_stun_port, 0, public_addr, NULL) < 0) {
		nice_stun_server.clear();
		return -1;
	}
	
	if(public_addr.ToString().empty()) {
		RTC_LOG(LERROR) << "Could not resolve public address...";
		nice_stun_server.clear();
		return -1;
	}
	public_address = public_addr.ToString();
	RTC_LOG(INFO) << "  >> Our public address is " << public_addr.ToString();	
	return 0;
}

int Handle::setTurnServer(
	const std::string& turn_server, uint16_t turn_port, const std::string& turn_type, 
	const std::string& turn_user, const std::string& turn_pwd) 
{
	if(turn_server.empty())
		return 0;	/* No initialization needed */
	if(turn_port == 0)
		turn_port = 3478;
	
	if (turn_type.empty()) {
		nice_turn_type = NICE_RELAY_TYPE_TURN_UDP;
	} else if(!strcasecmp(turn_type.c_str(), "udp")) {
		nice_turn_type = NICE_RELAY_TYPE_TURN_UDP;
	} else if(!strcasecmp(turn_type.c_str(), "tcp")) {
		nice_turn_type = NICE_RELAY_TYPE_TURN_TCP;
	} else if(!strcasecmp(turn_type.c_str(), "tls")) {
		nice_turn_type = NICE_RELAY_TYPE_TURN_TLS;
	} else {
		RTC_LOG(LERROR) << "Unsupported relay type " << turn_type;
		return -1;
	}

	RTC_LOG(INFO) << "TURN server to use: "
								<< turn_server << ":" << turn_port 
								<< "(" << nice_turn_type << ")";	
	
	/* Resolve address to get an IP */
	struct addrinfo *res = NULL;
	if(getaddrinfo(turn_server.c_str(), NULL, NULL, &res) != 0 ) {
		RTC_LOG(LERROR) << "Could not resolve " << turn_server;
		if(res)
			freeaddrinfo(res);
		return -1;
	}	
	network_address addr;
	struct sockaddr *address = res->ai_addr;
	if (address != NULL) {
		if(address->sa_family == AF_INET) {
			struct sockaddr_in *addr_ = (struct sockaddr_in *)address;
			network_address temp(addr_->sin_addr);
			addr =  std::move(temp);
		} else if(address->sa_family == AF_INET6) {
			struct sockaddr_in6 *addr_ = (struct sockaddr_in6 *)address;
			network_address temp(addr_->sin6_addr);
			addr =  std::move(temp);
		}	
	}
	if (res) {
		freeaddrinfo(res);
		res = NULL;
	}

	if (!nice_turn_server.empty())
		nice_turn_server.clear();	
	nice_turn_server = addr.ToString();
	if(nice_turn_server.empty()) {
		RTC_LOG(LERROR) << "Could not resolve %s " << turn_server;
		return -1;
	}
	nice_turn_port = turn_port;

	RTC_LOG(LS_VERBOSE) << "  >> " << nice_turn_server << ":" << nice_turn_port;
	if(!turn_user.empty())
		nice_turn_user = turn_user;
	if(!turn_user.empty())
		nice_turn_pwd = turn_pwd;
	return 0;
}

int Handle::setTurnRestApi(
	const std::string& api_server, const std::string& api_key, const std::string& api_method) 
{
#ifndef HAVE_TURNRESTAPI
	RTC_LOG(LERROR) << "gatway has been built with no libcurl support, TURN REST API unavailable";
	return -1;
#else
	if(api_server.emptr() &&
			(strstr(api_server.c_str(), "http://") != api_server.c_str() 
				&& strstr(api_server.c_str(), "https://") != api_server.c_str())) {
		RTC_LOG(LERROR) << "Invalid TURN REST API backend: not an HTTP address";
		return -1;
	}
#if 0	
	janus_turnrest_set_backend(api_server, api_key, api_method);
	JANUS_LOG(LOG_INFO, "TURN REST API backend: %s\n", api_server ? api_server : "(disabled)");
#endif	
#endif
	return 0;
}
/**
 * 
 */ 
void Handle::debugEnable()
{
	RTC_LOG(INFO) << "Enabling libnice debugging...";
	
	if(g_getenv("NICE_DEBUG") == NULL) {
		RTC_LOG(WARNING) << "No NICE_DEBUG environment variable set, setting maximum debug";
		g_setenv("NICE_DEBUG", "all", TRUE);
	}
	if(g_getenv("G_MESSAGES_DEBUG") == NULL) {
		RTC_LOG(WARNING) << "No G_MESSAGES_DEBUG environment variable set, setting maximum debug";
		g_setenv("G_MESSAGES_DEBUG", "all", TRUE);
	}
	
	rtc::StringBuilder stringBuilder;
	stringBuilder.AppendFormat("Debugging NICE_DEBUG=%s G_MESSAGES_DEBUG=%s\n",
			g_getenv("NICE_DEBUG"), g_getenv("G_MESSAGES_DEBUG"));
	
	RTC_LOG(LS_VERBOSE) << stringBuilder.str();
	stringBuilder.Clear();
	nice_debugging_enabled = TRUE;
	nice_debug_enable(strstr(g_getenv("NICE_DEBUG"), "all") || strstr(g_getenv("NICE_DEBUG"), "stun"));

}
/**
 * 
 */ 
void Handle::debugDisable()
{
	RTC_LOG(LS_VERBOSE) << "Disabling libnice debugging...";
	nice_debugging_enabled = FALSE;
	nice_debug_disable(TRUE);
}

Handle::Handle (
	gateway_session& core_session,
	const std::string opaque_id)
	: session(core_session),
		handle_id(0),
	  app(nullptr),
	  app_session(nullptr)
{
	RTC_DCHECK(session);		
	RTC_DCHECK(session->gateway_p);
	gateway* gateway_p = (gateway*)session->gateway_p;
	while(handle_id == 0) {
		handle_id = gateway_random_uint64();
		if (gateway_p->gatewaySessionFindHandles(
				session, handle_id) != nullptr)
			handle_id = 0;
	}
	this->opaque_id = opaque_id;	
	this->created = rtc::SystemTimeMillis();
}

Handle::~Handle()
{
	RTC_LOG(INFO) << "start:" << __FUNCTION__;
	session = nullptr;
	app_session = nullptr;
	app = nullptr;	
}

/**
 * attach plugin
 */ 
int Handle::createSession(const gateway_plugin& plugin)
{
	//RTC_LOG(INFO) << "start:" << __FUNCTION__ << getType();
	if(session == nullptr)
		return GATEWAY_ERROR_SESSION_NOT_FOUND;
	if(plugin == nullptr)
		return GATEWAY_ERROR_PLUGIN_NOT_FOUND;
	if(app != nullptr) {
		/* This handle is already attached to a plugin */
		return GATEWAY_ERROR_PLUGIN_ATTACH;
	}
	int error = 0;
	app = plugin;
	//gateway_handle g_handle = nullptr;
	switch (getType()) {
		case GATEWAY_HANDLE_P2P:
			//g_handle = static_cast<P2pHandle*>(this);
			app_session = app->createSession(static_cast<P2pHandle*>(this),&error);
			break;
		case GATEWAY_HANDLE_RELAY:	
			//g_handle = static_cast<NiceHandle*>(this);
			app_session = app->createSession(static_cast<NiceHandle*>(this),&error);
			break;
		default:
			break;
	}
	
	if(error !=0 || app_session == nullptr) {
		RTC_LOG(LERROR) << "plugin create sesssion error" ;
		/* TODO Make error struct to pass verbose information */
		app_session = nullptr;
		return error;
	}
	
	atomic_int_set(&app_session->stopped, 0);	
	/* Add this plugin session to active sessions map */
	{
		rtc::CritScope lock(&plugin_sessions_mutex);
		plugin_sessions.insert(
			std::pair<gateway_plugin_session,gateway_plugin_session>
			(app_session,app_session)
		);	
	}
	atomic_int_set(&destroyed,0);
	return 0;	
}

/**
 * destory *
 */ 
int Handle::destorySession()
{
	RTC_LOG(INFO) << "start:" << __FUNCTION__;
	int error;
	if(session == nullptr)
		return GATEWAY_ERROR_SESSION_NOT_FOUND;
	if(app_session == nullptr)
		return GATEWAY_ERROR_PLUGIN_DETACH;	
#if 1	
	if(!atomic_int_compare_and_exchange(&destroyed, 0, 1))
		return 0;
#endif
	/* Remove the session from active sessions map */
	{
		rtc::CritScope lock(&plugin_sessions_mutex);
		auto iterator = 
			plugin_sessions.find(app_session);
		if (iterator != plugin_sessions.end()) {
			RTC_LOG(INFO) << "Found active session in plugin_sessions" ;
			iterator->second = nullptr;
			plugin_sessions.erase(iterator);
		}
	}
	app->destorySession(app_session,&error);
	app_session = nullptr;
	app = nullptr;
	return 0;
}

bool Handle::sessionIsAlive(const gateway_plugin_session& session) 
{
	if(session == NULL 
		|| atomic_int_get(&session->stopped))
		return false;
	/* Make sure this plugin session is still alive */
	rtc::CritScope lock(&plugin_sessions_mutex);
	
	if (plugin_sessions.empty()) 
		return false;

	auto iterator = plugin_sessions.find(session);
	if (iterator != plugin_sessions.end() 
					&& iterator->second != nullptr)
		return true;
	
	RTC_LOG(LERROR) << "Invalid plugin session ");

	return false;
}
