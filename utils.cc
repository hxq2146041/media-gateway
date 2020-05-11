
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "rtc_base/checks.h"
#include "rtc_base/helpers.h"
#include "rtc_base/logging.h"


void gateway_get_json_type_name(int type, unsigned int flags, char *type_name) 
{
	/* Longest possible combination is "a non-empty boolean" plus one for null char */
	size_t req_size = 20;
	/* Don't allow for both "positive" and "non-empty" because that needlessly increases the size. */
	if((flags & GATEWAY_JSON_PARAM_POSITIVE) != 0) {
		strncpy(type_name, "a positive ", req_size);
	}
	else if((flags & GATEWAY_JSON_PARAM_NONEMPTY) != 0) {
		strncpy(type_name, "a non-empty ", req_size);
	}
	else if(type == json_int || type == json_uint 
		|| type == json_array || type == json_object) {
		strncpy(type_name, "an ", req_size);
	}
	else {
		strncpy(type_name, "a ", req_size);
	}
	
	switch(type) {
		case json_bool:
			strncat(type_name, "boolean", req_size);
			break;
		case json_int:
		case json_uint:	
			strncat(type_name, "integer", req_size);
			break;
		case json_double:
			strncat(type_name, "real", req_size);
			break;
		case json_string:
			strncat(type_name, "string", req_size);
			break;
		case json_array:
			strncat(type_name, "array", req_size);
			break;
		case json_object:
			strncat(type_name, "object", req_size);
			break;
		default:
			break;
	}
}

bool json_value_type_valid(
	const json_value& val, json_type type, unsigned int flags) 
{
	bool is_valid = (val.type() == type);
	if((flags & GATEWAY_JSON_PARAM_POSITIVE) != 0) {
		switch(type) {
			case json_int: {
				if (val.isInt()) {
					int value;
					rtc::GetIntFromJson(val,&value);
					is_valid = (value >= 0);
				} else if (val.isInt64()) {
					int64_t value;
					rtc::GetInt64FromJson(val,&value);
					is_valid = (value >= 0);
				}
				break;
			}
			case json_uint: {
				if (val.isUInt()) {
					uint32_t value;
					rtc::GetUIntFromJson(val,&value);
					is_valid = (value >= 0);
				} else if (val.isUInt64()) {
					uint64_t value;
					rtc::GetUInt64FromJson(val,&value);
					is_valid = (value >= 0);
				}
				break;
			}	
			case json_double: {
				double value = 0;
				if (val.isDouble())
					rtc::GetDoubleFromJson(val,&value);
				is_valid = (value >= 0);
				break;
			}
			default: {
				break;
			}
		}
	}
	
	else if((flags & GATEWAY_JSON_PARAM_NONEMPTY) != 0) {
		switch(type) {
			case json_string: {
				std::string value;
				rtc::GetStringFromJson(val,&value);
				is_valid = (strlen(value.c_str()) > 0);
				break;
			}
			case json_array: {
				if (val.isArray())
					return false;
				std::vector<Json::Value> value;
				rtc::JsonArrayToValueVector(val,&value);
				is_valid = (value.size() > 0);
				break;
			}

			default: {
				break;
			}
		}
	}
	return is_valid;
}


std::string gateway_random_id_string()
{
	uint32_t number = rtc::CreateRandomId();
	std::stringstream st;
	st << number;
	return st.str();
}

uint64_t* gateway_uint64_dup(uint64_t num) 
{
	uint64_t *numdup = 
		static_cast<uint64_t*>(malloc(sizeof(uint64_t)));
	*numdup = num;
	return numdup;
}

uint32_t gateway_random_uint32(void) 
{
	return rtc::CreateRandomId();
}

uint64_t gateway_random_uint64(void) 
{
	uint64_t num = rtc::CreateRandomId() & 0x1FFFFF;
	num = (num << 32) | rtc::CreateRandomId();
	return num;
}


