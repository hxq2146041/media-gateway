#ifndef GATEWAY_CONFIG_H
#define GATEWAY_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
#include <libconfig.h>

typedef config_t gateway_cfg;

typedef enum gateway_cfg_type {
	gateway_cfg_type_general = 1,
	gateway_cfg_type_admin,
	gateway_cfg_type_certificates,
	gateway_cfg_type_media,
	gateway_cfg_type_events,	
	gateway_cfg_type_nat,
	gateway_cfg_type_butt,
}gateway_cfg_type;

const char* gateway_cfg_type_to_string(gateway_cfg_type type);

gateway_cfg* gateway_cfg_parse(/*const char* path,*/const char *config_file);

gateway_cfg* gateway_cfg_create(const char* path,const char *name);

void gateway_cfg_destory(gateway_cfg* cfg);

int gateway_cfg_get_int(
	const gateway_cfg* cfg,gateway_cfg_type type,const char* item,int* value);

int gateway_cfg_set_int(gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,int value);


int gateway_cfg_get_int_64(
	const gateway_cfg* cfg,gateway_cfg_type type,const char* item,long long* value);

int gateway_cfg_set_int64(gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,long long value);


int gateway_cfg_get_string(
	const gateway_cfg* cfg,gateway_cfg_type type,const char* item,const char** value);

int gateway_cfg_set_string(gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,const char *value);


int gateway_cfg_get_float(
	const gateway_cfg* cfg,gateway_cfg_type type,const char* item,double* value);

int gateway_cfg_get_bool(
	const gateway_cfg* cfg,gateway_cfg_type type,char* item,bool* value);

int gateway_cfg_set_bool(gateway_cfg* cfg,
	gateway_cfg_type type,
	const char* item,int value);

#endif
