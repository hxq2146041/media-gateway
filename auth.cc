#include "auth.h"
#include "jwt/jwt.h"
#include <string.h>
#include "mutex.h"
#include "absl/memory/memory.h"
#include "rtc_base/logging.h"

static bool auth_enabled = false;
static unsigned char* auth_secret = NULL;
static gateway_mutex mutex;

/* Setup */
void gateway_auth_init(bool enabled, unsigned char* secret) 
{
	if(enabled && secret) {
		auth_secret = 
				(unsigned char*)strdup((char*) secret);
		auth_enabled = true;
	}
	//gateway_mutex_init(&mutex);
}

bool gateway_auth_is_enabled(void) 
{
	return auth_enabled;
}

void gateway_auth_deinit(void) 
{
	//gateway_mutex_lock(&mutex);
	if (auth_secret)
		free(auth_secret);
	auth_secret = NULL;
	//gateway_mutex_unlock(&mutex);
}

static std::unique_ptr<JwtToken> 
	gateway_auth_check_signature(const char *token)
{
	if (!auth_enabled || auth_secret == NULL || token == NULL)
		return nullptr;
	std::unique_ptr<JwtToken> jwt = 
		absl::make_unique<JwtToken>(token,auth_secret);
	if (jwt->DecodeAndVerify() != 0) {
		RTC_LOG(LERROR) << "DecodeAndVerify Error";
		return nullptr;
	}
	if (!jwt->VerifyExpire()) {
		RTC_LOG(LERROR) << "Token Expire Error";
		return nullptr;
	}
	
	if (!jwt->VerifyInfo("iss","gateway")) {
		RTC_LOG(LERROR) << "iss Error";		
		return nullptr;	
	}
	return std::move(jwt);
}

bool gateway_auth_check_token(
	const char *token,const char* grant,const char* val) 
{	
	bool ret =false;
	/* Always TRUE if the mechanism is disabled, of course */
	if(!auth_enabled)
		return true;
	if (grant == NULL || val == NULL)
		return (gateway_auth_check_signature(token) != nullptr);
	std::unique_ptr<JwtToken> jwt = 
		gateway_auth_check_signature(token);
	if (jwt == nullptr)
		return false;
	if (!jwt->VerifyInfo(grant,val))
		return false;	

	return true;
}

bool gateway_auth_check_contains(
	const char *token,const char* grant,const char* val) 
{	
	bool ret =false;
	/* Always TRUE if the mechanism is disabled, of course */
	if(!auth_enabled)
		return true;
	if (grant == NULL || val == NULL)
		return (gateway_auth_check_signature(token) != nullptr);
	std::unique_ptr<JwtToken> jwt = 
		gateway_auth_check_signature(token);
	if (jwt == nullptr)
		return false;
	
	if (!jwt->VerifyContains(grant,val))
		return false;	

	return true;
}

