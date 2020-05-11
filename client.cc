#include <iostream>
#include <string>
#include <vector>

#include "absl/strings/str_join.h"
#include "absl/algorithm/container.h"
#include "rtc_base/string_to_number.h"
#include "rtc_base/logging.h"
#include "rtc_base/thread.h"
#include "rtc_base/thread_pool.h"
#include "system_wrappers/include/clock.h"
#include "sdk/gateway_client.h"

class GatewayCallback : public IGatewayCallback {
 public:
 	GatewayCallback() = default;
	
	~GatewayCallback() = default ; 	

  void onSuccess() override;

  void onDestroy() override;

  void onError(const std::string& error) override;	
};

void GatewayCallback::onSuccess() 
{

}

void GatewayCallback::onDestroy() 
{

}

void GatewayCallback::onError(const std::string& error)
{

}

int main()
{
	rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE);
	rtc::LogMessage::SetLogToStderr(true);
	rtc::LogMessage::LogTimestamps(true);

	//Gateway gateway_;
	gateway_client client(new GatewayCallback());
	client.callP2p("45678",std::string());
  while(true);
		
	return 0;  
}
