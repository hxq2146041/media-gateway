

#include "plugin.h"
#include "p2p_call.h"
#include "rtc_base/logging.h"

/* Plugin results */
gateway_plugin_result gateway_plugin_result_new(
	gateway_plugin_result_type type, 
	const std::string& text, 
	const json_value& content) 
{
	RTC_LOG(LS_VERBOSE) <<  "Creating plugin result...";
	gateway_plugin_result result = 
			new rtc::RefCountedObject<PluginResult>(); 
	result->type = type;
	result->text = text;
	result->content = content;
	return result;
}

void gateway_plugin_result_destroy(gateway_plugin_result& result) 
{
	RTC_LOG(LS_VERBOSE) <<  "Destroying plugin result...";
	if (result == nullptr) return;
	if (!result->text.empty())
		result->text.clear();
	if (!result->content.empty())
		result->content.clear();	
}

gateway_plugin createPlugin(
	PluginCallbacks* callback,PluginType type/*,const std::string& cfg*/)
{
	switch (type) {
		case GATEWAY_PLUGIN_P2PCALL:
			return new rtc::RefCountedObject<P2pCall>(callback/*,cfg*/); 
		default:
			break;
	}
	return nullptr;
}

