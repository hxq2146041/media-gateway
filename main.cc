#include <iostream>
#include <string>
#include <vector>
#include <sys/time.h>
#include <sys/resource.h>

#include "absl/strings/str_join.h"
#include "absl/algorithm/container.h"
#include "rtc_base/string_to_number.h"
#include "rtc_base/logging.h"
#include "rtc_base/thread.h"
#include "rtc_base/thread_pool.h"
#include "system_wrappers/include/clock.h"
#include "config.h"
#include "gateway.h"
#include "jwt/jwt.h"

class Test : public rtc::RefCountInterface {
 public:
 	Test() {

		RTC_LOG(INFO) << "#################";
		value = 1024;
	}
 	virtual ~Test() {RTC_LOG(INFO) << "@@@@@@@@@@@@@@@@@@@@@@@@@";}
	virtual bool HasOneRef() const = 0;
	int value;
};

void test_func(rtc::scoped_refptr<Test>& test)
{
	if (test == nullptr)
		RTC_LOG(INFO) << "@start :" << __FUNCTION__;
	else
		RTC_LOG(INFO) << "#start :" <<  test->value;
}

class AA {
 public:
 	AA(void* p) 
	{
		session = p;
		test();
	}  
	void test() 
	{
		rtc::scoped_refptr<Test> t(static_cast<Test*>(session));
		test_func(t);
	}
 private:
 	void* session;
};

static const char* t_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhZG1pbiI6ImZhbHNlIiwiYXVkIjoiY3VzdG1lciIsImV4cCI6MTkwNDQ3MDQ2MDAwMCwiaXNzIjoiZ2F0ZXdheSIsInBsdWdpbiI6InAycCIsInN1YiI6ImRldmVsb3BlciJ9.kpS24OCL1mPhDb38Sw4pr-LAwcR71sP9nTQpkIH8Cvk";

void test_token()
{
	
	const char* key_file = "3392122_ubonass.com.key";
	unsigned char out[20000];
	int length = jwt_read_key(key_file,out);
	unsigned char* key = (unsigned char*)alloca(length + 1);
	memcpy(key,out,length);
	key[length] = '\0';

	std::unique_ptr<JwtToken> jwt = 
		absl::make_unique<JwtToken>(
				JwtToken::Builder()
						.add_alg(JWT_ALG_HS256,key)
						.add_grant("iss","gateway")
						.add_grant("sub","developer")
						.add_grant("aud","custmer")
						.add_grant("admin","false")
						.add_grant("plugin","p2pcall.videocall")
						.add_grant_int64("exp",1904470460000)
						.build());//2030/5/8 19:34:20

	jwt->EncodeAndSignature();

	printf("token:%s\n",jwt->token().c_str());

	//RTC_LOG(INFO) << "token:" << jwt->token();

	std::unique_ptr<JwtToken> jwt2 = 
		absl::make_unique<JwtToken>(t_token,key);

	int ret = jwt2->DecodeAndVerify();
	if (ret == 0)
		RTC_LOG(INFO) << "Verify success:";
	else {
		RTC_LOG(INFO) << "Verify error:";
	}
	bool ret2 = jwt2->VerifyExpire();
	if (ret2)
		RTC_LOG(INFO) << "Verify success:";
	else {
		RTC_LOG(INFO) << "@@@@Verify error:";
	}
}

void scoped_refptr_test()
{
	rtc::scoped_refptr<Test> test = 
				new rtc::RefCountedObject<Test>();	
	AA a((void *)test.get());
	a.test();
}

int main(int argc, char *argv[]) {
	//rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
	//rtc::LogMessage::SetLogToStderr(true);
	//rtc::LogMessage::LogTimestamps(true);
	//test_token();
	args_info_t args_info;
	/* Let's call our cmdline parser */
	if(cmdline_parser(argc, argv, &args_info) != 0)
		exit(1);
	return gateway::StartArgs(args_info);
}
