#ifndef GATEWAY_UTILS_H
#define GATEWAY_UTILS_H

#include "helper.h"

typedef struct gateway_json_parameter gateway_json_parameter;
struct gateway_json_parameter {
	const char *name;
	json_type type;
	unsigned int flags;
};

void gateway_get_json_type_name(int type, unsigned int flags, char *type_name); 

bool json_value_type_valid(const json_value& val, json_type type, unsigned int flags); 

#define GATEWAY_VALIDATE_JSON_OBJECT_FORMAT(missing_format, invalid_format, obj, params, error_code, error_cause, log_error, missing_code, invalid_code) \
	do { \
		error_code = 0; \
		unsigned int i; \
		for(i = 0; i < sizeof(params) / sizeof(struct gateway_json_parameter); i++) { \
			if(!obj.isMember(params[i].name)) { \
				if((params[i].flags & GATEWAY_JSON_PARAM_REQUIRED) != 0) {	\
					error_code = (missing_code); \
					if(error_cause != NULL) \
						snprintf(error_cause, sizeof(error_cause), missing_format, params[i].name); \
					break; \
				} \
				continue; \
			} \
			json_value value = obj[params[i].name]; \
			if(!json_value_type_valid(value, params[i].type, params[i].flags)) { \
				error_code = (invalid_code); \
				char type_name[20]; \
				gateway_get_json_type_name(params[i].type, params[i].flags, type_name); \
				if(error_cause != NULL) \
					snprintf(error_cause, sizeof(error_cause), invalid_format, params[i].name, type_name); \
				break; \
			} \
		} \
	} while(0)

#define GATEWAY_VALIDATE_JSON_OBJECT(obj, params, error_code, error_cause, log_error, missing_code, invalid_code) \
	GATEWAY_VALIDATE_JSON_OBJECT_FORMAT("Missing mandatory element (%s)", "Invalid element type (%s should be %s)", obj, params, error_code, error_cause, log_error, missing_code, invalid_code)


std::string gateway_random_id_string();

uint64_t* gateway_uint64_dup(uint64_t num);

uint32_t gateway_random_uint32(void);

uint64_t gateway_random_uint64(void);


#endif
