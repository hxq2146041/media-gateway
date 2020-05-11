#ifndef GATEWAY_HANDLE_H
#define GATEWAY_HANDLE_H

#include "absl/memory/memory.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/critical_section.h"

#include "plugins/plugin.h"
#include "helper.h"

extern "C" {
#include <glib.h>
#include <agent.h>
#include <stun/stunagent.h>
#include <stun/usages/bind.h>
} 

enum HandleType {
	GATEWAY_HANDLE_P2P,
	GATEWAY_HANDLE_RELAY	
};

class Handle 
	: public rtc::RefCountInterface ,
		public sigslot::has_slots<> {
 public:
 	explicit Handle(
		gateway_session& core_session,
		const std::string opaque_id);
  virtual ~Handle();
	
	virtual bool HasOneRef() const = 0;
	
	static void init(
		bool ice_lite, bool ice_tcp, bool full_trickle, bool ignore_mdns,
		bool ipv6, uint16_t rtp_min_port, uint16_t rtp_max_port); 	

	static void deInit(); 

	/**
	 * test stun server
	 */
	static int testStunServer(
		network_address& addr, uint16_t port,uint16_t local_port, 
		network_address& public_addr, uint16_t *public_port); 	
	/**
	 * set stun server
	 * public_addr:return public_addr is success
	 */	
	static int setStunServer(
		const std::string& stun_server, uint16_t stun_port);
	/**
	 * set turn server
	 */	
	static int setTurnServer(
		const std::string& turn_server, uint16_t turn_port, const std::string& turn_type, 
		const std::string& turn_user, const std::string& turn_pwd); 	
	/**
	 * set turn server
	 */	
	static int setTurnRestApi(
		const std::string& api_server, const std::string& api_key, 
		const std::string& api_method); 
	/**
	 * 
	 */	
	static void debugEnable();
	/**
	 * 
	 */	
	static void debugDisable();
	/**
	 * 
	 */	
	virtual HandleType getType() = 0;
	/**
	 * create plugin session
	 */	
	virtual int createSession(const gateway_plugin& plugin);		
	/**
	 * destory*
	 */	
	virtual int destorySession();	
	/**
	 * create NiceAgent *
	 */	
	virtual void createAgent(int offer) = 0;

	/**
	 * sessionIsAlive 
	 * @ session
	 */	
	bool sessionIsAlive(
		const gateway_plugin_session& session);
			
	/* RTP/RTCP port range */
	static uint16_t rtp_range_min;
	/* RTP/RTCP port range */
	static uint16_t rtp_range_max;	
	/* ICE-Lite status */
	static bool nice_lite_enabled;
	/* ICE-TCP support (only libnice >= 0.1.8, currently broken) */
	static bool nice_tcp_enabled;
	/* Full-trickle support */
	static bool nice_full_trickle_enabled;
	/* mDNS resolution support */
	static bool nice_mdns_enabled;
	/* IPv6 support (still mostly WIP) */
	static bool nice_ipv6_enabled;
	/* STUN server/port, if any */
	static std::string nice_stun_server;
	static uint16_t nice_stun_port;	
	static std::string public_address;
	/* TURN server/port and credentials, if any */
	static std::string nice_turn_server;
	static uint16_t nice_turn_port;
	static std::string nice_turn_user;
	static std::string nice_turn_pwd;
	static NiceRelayType nice_turn_type;	

	static bool nice_debugging_enabled;

  // Called whenever an RTCP packet is received. There is no equivalent signal
  // for RTP packets because they would be forwarded to the BaseChannel through
  // the RtpDemuxer callback.
  sigslot::signal2<rtc::CopyOnWriteBuffer*, int64_t> SignalRtcpPacketReceived;

	/* Map of active plugin sessions */
	static std::map<gateway_plugin_session,
						gateway_plugin_session> plugin_sessions;
	
	static rtc::CriticalSection plugin_sessions_mutex;

	/*! \brief Opaque pointer to the core/peer session */
	gateway_session session;
	/*! \brief Handle identifier, guaranteed to be non-zero */
	uint64_t handle_id;	
	/*! \brief Opaque identifier, e.g., to provide inter-handle relationships to external tools */
	std::string opaque_id;	
	/*! \brief Monotonic time of when the handle has been created */
	int64_t created;		
	/*! \brief Opaque application (plugin) pointer */
	gateway_plugin app;
	/*! \brief Opaque core/plugin session pointer */
	gateway_plugin_session app_session;	
	/*! \brief Mutex to lock/unlock the ICE session */
	rtc::CriticalSection mutex;	
	/*! \brief Whether a close_pc was requested recently on the PeerConnection */
	volatile int closepc;
	/*! \brief Atomic flag to check if this instance has been destroyed */
	volatile int destroyed;	
};

#endif
