#include "config.h"
#include "rtc_base/logging.h"
#include "rtc_base/checks.h"

const char* 
	gateway_cfg_type_to_string(gateway_cfg_type type)
{
	const char* type_string = "";
	switch(type) {
		case gateway_cfg_type_general:
			type_string = "general";
			break;
		case gateway_cfg_type_admin:
			type_string = "admin";
			break;			
		case gateway_cfg_type_certificates:
			type_string = "certificates";
			break;	
		case gateway_cfg_type_media:
			type_string = "media";
			break;				
		case gateway_cfg_type_nat:
			type_string = "nat";
			break;				
		case gateway_cfg_type_events:
			type_string = "events";
			break;		
		default:
			break;
	}
	return type_string;
}

gateway_cfg* gateway_cfg_parse(
	/*const char* path,*/const char *name)
{
	if (name == NULL) {
		RTC_LOG(LERROR) << "NOT FOUND CFG FILE";
		return NULL;
	}
  gateway_cfg* cfg = 
			(gateway_cfg*)malloc(sizeof(gateway_cfg));
  if (cfg == NULL) {
    RTC_LOG(LERROR) << "malloc gateway_cfg error";
    return NULL;
  }
	config_init(cfg);
	#if 0
	if (path == NULL || !strcasecmp("",path))
		config_set_include_dir(cfg, "./");
	else
		config_set_include_dir(cfg, path);
	#endif
  int result = config_read_file(cfg, name);
  RTC_DCHECK(result);
	return cfg;
}

gateway_cfg* gateway_cfg_create(const char* path,const char *name)
{
	return NULL;
}

void gateway_cfg_destory(gateway_cfg* cfg)
{
	if (cfg == NULL) return;
	config_destroy(cfg);
}

int gateway_cfg_set_int(gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,int value)
{
	if (cfg == NULL )
		return CONFIG_FALSE;
	RTC_DCHECK(item != NULL);
	RTC_DCHECK(type != gateway_cfg_type_butt);
	config_setting_t* parent = 
			config_lookup(cfg, gateway_cfg_type_to_string(type));
	
	std::string member = gateway_cfg_type_to_string(type);
	if (member.empty())
		return CONFIG_FALSE;
	member.append(".");
	member.append(item);
  config_setting_t* setting = 
				config_lookup(cfg, member.c_str());
	if (setting == NULL) 
		setting = 
			config_setting_add(
				parent, item, CONFIG_TYPE_INT);
	
  return config_setting_set_int(setting, value);
}

int gateway_cfg_get_int(
	const gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,int* value)
{
	if (cfg == NULL )
		return CONFIG_FALSE;
	RTC_DCHECK(item != NULL);
	RTC_DCHECK(type != gateway_cfg_type_butt);
	std::string path = gateway_cfg_type_to_string(type);
	if (path.empty())
		return -1;
	path.append(".");
	path.append(item);
	return config_lookup_int(cfg,path.c_str(),value);	 
}

int gateway_cfg_set_int64(gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,long long value)
{
	if (cfg == NULL )
		return CONFIG_FALSE;
	RTC_DCHECK(item != NULL);
	RTC_DCHECK(type != gateway_cfg_type_butt);
	config_setting_t* parent = 
				config_lookup(cfg, gateway_cfg_type_to_string(type));

	std::string member = gateway_cfg_type_to_string(type);
	if (member.empty())
		return CONFIG_FALSE;
	member.append(".");
	member.append(item);
  config_setting_t* setting = 
				config_lookup(cfg, member.c_str());
	if (setting == NULL) 
		setting = 
			config_setting_add(
				parent, item, CONFIG_TYPE_INT64);

  return config_setting_set_int64(setting, value);
}


int gateway_cfg_get_int_64(
	const gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,long long* value)
{
	if (cfg == NULL )
		return CONFIG_FALSE;

	RTC_DCHECK(item != NULL);
	RTC_DCHECK(type != gateway_cfg_type_butt);
	std::string path = gateway_cfg_type_to_string(type);
	if (path.empty())
		return -1;
	path.append(".");
	path.append(item);
	return config_lookup_int64(cfg,path.c_str(),value);	 
}


int gateway_cfg_set_string(gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,const char *value)
{
	if (cfg == NULL )
		return CONFIG_FALSE;
	RTC_DCHECK(item != NULL);
	RTC_DCHECK(type != gateway_cfg_type_butt);
	
	config_setting_t* parent = 
			config_lookup(cfg, gateway_cfg_type_to_string(type));
	
	std::string member = gateway_cfg_type_to_string(type);
	if (member.empty())
		return CONFIG_FALSE;
	member.append(".");
	member.append(item);
  config_setting_t* setting = 
				config_lookup(cfg, member.c_str());
	if (setting == NULL) 
		setting = 
			config_setting_add(
				parent, item, CONFIG_TYPE_STRING);

  return config_setting_set_string(setting, value);
}


int gateway_cfg_get_string(
	const gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,const char** value)
{
	if (cfg == NULL )
		return CONFIG_FALSE;

	RTC_DCHECK(item != NULL);
	RTC_DCHECK(type != gateway_cfg_type_butt);
	std::string path = gateway_cfg_type_to_string(type);
	if (path.empty())
		return -1;
	path.append(".");
	path.append(item);
	return config_lookup_string(cfg,path.c_str(),value);
}

int gateway_cfg_get_float(
	const gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,double* value)
{
	int result = 0;
	if (cfg == NULL )
		return CONFIG_FALSE;

	RTC_DCHECK(item != NULL);
	RTC_DCHECK(type != gateway_cfg_type_butt);
	std::string path = gateway_cfg_type_to_string(type);
	if (path.empty())
		return -1;
	path.append(".");
	path.append(item);
	return config_lookup_float(cfg,path.c_str(),value);	 
}

int gateway_cfg_get_bool(
	const gateway_cfg* cfg,
	gateway_cfg_type type,
	char* item,bool* value)
{
	int result = 0;
	if (cfg == NULL )
		return CONFIG_FALSE;

	RTC_DCHECK(item != NULL);
	RTC_DCHECK(type != gateway_cfg_type_butt);
	
	std::string path = gateway_cfg_type_to_string(type);
	if (path.empty())
		return -1;
	path.append(".");
	path.append(item);
	int ivalue = 0;
	result = config_lookup_bool(cfg,path.c_str(),&ivalue);
	*value = ivalue > 0 ? true : false;
	return result;
}

int gateway_cfg_set_bool(gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,int value)
{
	if (cfg == NULL )
		return CONFIG_FALSE;
	
	RTC_DCHECK(item != NULL);
	RTC_DCHECK(type != gateway_cfg_type_butt);
	
	config_setting_t* parent = 
			config_lookup(cfg, gateway_cfg_type_to_string(type));
	
	std::string member = gateway_cfg_type_to_string(type);
	if (member.empty())
		return CONFIG_FALSE;
	member.append(".");
	member.append(item);
  config_setting_t* setting = 
				config_lookup(cfg, member.c_str());
	
	if (setting == NULL) 
		setting = 
			config_setting_add(
				parent, item, CONFIG_TYPE_BOOL);

  return config_setting_set_bool(setting, value);

}

