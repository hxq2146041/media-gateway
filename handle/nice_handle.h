#ifndef GATEWAY_NICE_HANDLE_H
#define GATEWAY_NICE_HANDLE_H

#include "handle.h"

class NiceHandle : public Handle {
 public:
 	explicit NiceHandle(
		gateway_session& core_session,const std::string opaque_id);
  virtual ~NiceHandle();

	void onCandidateGatheringGone(NiceAgent *agent, guint stream_id); 
	
	void onComponentStateChanged(
			NiceAgent *agent, guint stream_id, guint component_id, guint state); 
	
	void onNewSelectedPair(
		NiceAgent *agent, guint stream_id, guint component_id, gchar *local, gchar *remote);

	void onNewLocalCandidate(
		NiceAgent *agent, guint stream_id, guint component_id, gchar *foundation);
	
	void onNewRemoteCandidate(
		NiceAgent *agent, guint stream_id, guint component_id, gchar *foundation); 

	void onNewSelectedPair(
		NiceAgent *agent, guint stream_id, guint component_id, NiceCandidate *local, NiceCandidate *remote); 

	void onNewLocalCandidate(NiceAgent *agent, NiceCandidate *candidate);

	void onNewRemoteCandidate(NiceAgent *agent, NiceCandidate *candidate); 
	
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
	 void createAgent(int offer) override;

 private:
	/*! \brief Opaque application (plugin) pointer */
	//gateway_plugin plugin_;
	/*! \brief Opaque core/plugin session pointer */
	//gateway_plugin_session plugin_session_;	
	/*! \brief libnice ICE agent */
	NiceAgent *agent;
	/*! \brief Monotonic time of when the ICE agent has been created */
	int64_t agent_created; 	
	/*! \brief ICE role (controlling or controlled) */
	bool controlling;	
	/*! \brief GLib context for the handle and libnice */
	GMainContext *mainctx;	
};
#endif
