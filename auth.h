#ifndef GATEWAY_AUTH_H_
#define GATEWAY_AUTH_H_

void gateway_auth_init(bool enabled, unsigned char* secret);
bool gateway_auth_is_enabled(void);
void gateway_auth_deinit(void);
bool gateway_auth_check_token(
	const char *token,const char* grant,const char* val);
bool gateway_auth_check_contains(
	const char *token,const char* grant,const char* val);

#endif
