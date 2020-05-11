#include "nice_handle.h"
#include "gateway.h"

/* Callbacks */
static void onCandidateGatheringGoneCallback(
	NiceAgent *agent, guint stream_id, gpointer user_data) 
{
	NiceHandle* handle = reinterpret_cast<NiceHandle*>(user_data);
	if (handle != NULL)
		handle->onCandidateGatheringGone(agent, stream_id);
}

static void onComponentStateChangedCallback(
	NiceAgent *agent, guint stream_id, guint component_id, guint state, gpointer user_data) 
{
	NiceHandle* handle = reinterpret_cast<NiceHandle*>(user_data);
	if (handle != NULL)
		handle->onComponentStateChanged(agent, stream_id,component_id,state);
}

#ifndef HAVE_LIBNICE_TCP
static void onNewSelectedPairCallback (
	NiceAgent *agent, guint stream_id, guint component_id, gchar *local, gchar *remote, gpointer user_data) 
{
	NiceHandle* handle = reinterpret_cast<NiceHandle*>(user_data);
	if (handle != NULL)
		handle->onNewSelectedPair(agent, stream_id,component_id,local,remote);
}

static void onNewLocalCandidateCallback (
	NiceAgent *agent, guint stream_id, guint component_id, gchar *foundation, gpointer user_data) 
{
	NiceHandle* handle = reinterpret_cast<NiceHandle*>(user_data);	
	if (handle != NULL)
		handle->onNewLocalCandidate(agent, stream_id,component_id,foundation);
}
static void onNewRemoteCandidateCallback (
	NiceAgent *agent, guint stream_id, guint component_id, gchar *foundation, gpointer user_data) 
{
		NiceHandle* handle = reinterpret_cast<NiceHandle*>(user_data);
		if (handle != NULL)
			handle->onNewRemoteCandidate(agent, stream_id,component_id,foundation);
}
#else
static void onNewSelectedPairCallback (
	NiceAgent *agent, guint stream_id, guint component_id, NiceCandidate *local, NiceCandidate *remote, gpointer user_data) 

{
	NiceHandle* handle = reinterpret_cast<NiceHandle*>(user_data);
	if (handle != NULL)
		handle->onNewSelectedPair(agent, stream_id,component_id,local,remote);
}

static void onNewLocalCandidateCallback (
	NiceAgent *agent, NiceCandidate *candidate, gpointer user_data) 

{
	NiceHandle* handle = reinterpret_cast<NiceHandle*>(user_data);
	if (handle != NULL)
		handle->onNewLocalCandidate(agent, candidate);
}

static void onNewRemoteCandidateCallback (
	NiceAgent *agent, NiceCandidate *candidate, gpointer user_data) 

{
	NiceHandle* handle = reinterpret_cast<NiceHandle*>(user_data);

	if (handle != NULL)
		handle->onNewRemoteCandidate(agent, candidate);
}
#endif

NiceHandle::NiceHandle(
	gateway_session& core_session,const std::string opaque_id)
	: Handle(core_session,opaque_id)
{
#if 0	
	handle->app = NULL;
	handle->app_handle = NULL;
	handle->queued_candidates = g_async_queue_new();
	handle->queued_packets = g_async_queue_new();
#endif	
}

NiceHandle::~NiceHandle()
{

}

/**
 * 
 */ 
HandleType NiceHandle::getType()
{
	return GATEWAY_HANDLE_RELAY;
}

/**
 * attach plugin
 */ 
int NiceHandle::createSession(const gateway_plugin& plugin)
{
	return 0;
}


void NiceHandle::createAgent(int offer)
{
	#if 1
	/* Note: NICE_COMPATIBILITY_RFC5245 is only available in more recent versions of libnice */
	controlling = nice_lite_enabled ? false : !offer;
	rtc::StringBuilder stringBuilder;
	stringBuilder.AppendFormat("[%lld] Creating ICE agent (ICE %s mode, %s)",
		handle_id,nice_lite_enabled ? "Lite" : "Full", controlling ? "controlling" : "controlled");
	RTC_LOG(INFO) << stringBuilder.str();
	stringBuilder.Clear();
	agent = (NiceAgent*)g_object_new(NICE_TYPE_AGENT,
		"compatibility", NICE_COMPATIBILITY_DRAFT19,
		"main-context", mainctx,
		"reliable", FALSE,
		"full-mode", nice_lite_enabled ? FALSE : TRUE,
#ifdef HAVE_LIBNICE_TCP
		"ice-udp", TRUE,
		"ice-tcp", nice_tcp_enabled ? TRUE : FALSE,
#endif
		NULL);
	agent_created = rtc::SystemTimeMillis();
#endif
	/* Any STUN server to use? */
	if(!nice_stun_server.empty() && nice_stun_port > 0) {
		g_object_set(G_OBJECT(agent),
			"stun-server", nice_stun_server,
			"stun-server-port", nice_stun_port,
			NULL);
	}
	/* Any dynamic TURN credentials to retrieve via REST API? */
	gboolean have_turnrest_credentials = FALSE;
#if 0	
#ifdef HAVE_TURNRESTAPI
	janus_turnrest_response *turnrest_credentials = janus_turnrest_request();
	if(turnrest_credentials != NULL) {
		have_turnrest_credentials = TRUE;
		JANUS_LOG(LOG_VERB, "[%"SCNu64"] Got credentials from the TURN REST API backend!\n", handle->handle_id);
		JANUS_LOG(LOG_HUGE, "  -- Username: %s\n", turnrest_credentials->username);
		JANUS_LOG(LOG_HUGE, "  -- Password: %s\n", turnrest_credentials->password);
		JANUS_LOG(LOG_HUGE, "  -- TTL:			%"SCNu32"\n", turnrest_credentials->ttl);
		JANUS_LOG(LOG_HUGE, "  -- Servers:	%d\n", g_list_length(turnrest_credentials->servers));
		GList *server = turnrest_credentials->servers;
		while(server != NULL) {
			janus_turnrest_instance *instance = (janus_turnrest_instance *)server->data;
			JANUS_LOG(LOG_HUGE, "  -- -- URI: %s:%"SCNu16" (%d)\n", instance->server, instance->port, instance->transport);
			server = server->next;
		}
	}
#endif
#endif
	g_object_set(G_OBJECT(agent), "upnp", FALSE, NULL);
	g_object_set(G_OBJECT(agent), "controlling-mode", controlling, NULL);
	g_signal_connect(G_OBJECT (agent), "candidate-gathering-done",
		G_CALLBACK(onCandidateGatheringGoneCallback),this);
	g_signal_connect(G_OBJECT (agent), "component-state-changed",
		G_CALLBACK(onComponentStateChangedCallback), this);
#ifndef HAVE_LIBNICE_TCP
	g_signal_connect(G_OBJECT (agent), "new-selected-pair",
#else
	g_signal_connect(G_OBJECT (agent), "new-selected-pair-full",
#endif
		G_CALLBACK(onNewSelectedPairCallback), this);
	if(nice_full_trickle_enabled) {
#ifndef HAVE_LIBNICE_TCP
		g_signal_connect(G_OBJECT (agent), "new-candidate",
#else
		g_signal_connect(G_OBJECT (agent), "new-candidate-full",
#endif
			G_CALLBACK(onNewLocalCandidateCallback), this);
	}
#ifndef HAVE_LIBNICE_TCP
	g_signal_connect(G_OBJECT (agent), "new-remote-candidate",
#else
	g_signal_connect(G_OBJECT (agent), "new-remote-candidate-full",
#endif
		G_CALLBACK(onNewRemoteCandidateCallback), this);
	/* Add all local addresses, except those in the ignore list */
}

void NiceHandle::onCandidateGatheringGone(NiceAgent *agent, guint stream_id)
{
	
}

void NiceHandle::onComponentStateChanged(
		NiceAgent *agent, guint stream_id, guint component_id, guint state)
{

}

void NiceHandle::onNewSelectedPair(
	NiceAgent *agent, guint stream_id, guint component_id, gchar *local, gchar *remote)
{

}

void NiceHandle::onNewLocalCandidate(
	NiceAgent *agent, guint stream_id, guint component_id, gchar *foundation)
{

}
void NiceHandle::onNewRemoteCandidate(
	NiceAgent *agent, guint stream_id, guint component_id, gchar *foundation)
{

}

void NiceHandle::onNewSelectedPair(
	NiceAgent *agent, guint stream_id, guint component_id, NiceCandidate *local, NiceCandidate *remote)
{

}

void NiceHandle::onNewLocalCandidate(NiceAgent *agent, NiceCandidate *candidate)
{

}

void NiceHandle::onNewRemoteCandidate(NiceAgent *agent, NiceCandidate *candidate)
{

}

