#include "transaction.h"

CreateSessionTransaction::CreateSessionTransaction(
	ICreateSessionCallbacks* callbacks) : callbacks_(callbacks) {}

CreateSessionTransaction::~CreateSessionTransaction()
{
	callbacks_ = NULL;
}

void CreateSessionTransaction::reportSuccess(const Json::Value& obj)
{
	std::string gateway_;
	if (!rtc::GetStringFromJsonObject(obj, "gateway",&gateway_)) {
		if (callbacks_)
			callbacks_->onError("Can't parse received message.");
		else 
			RTC_LOG(WARNING) << "Can't parse received message.";	
		return;
	}
	
	GatewayType type = gatewayTypeFromString(gateway_.c_str());
	if (type == GATEWAY_SUCCESS)
		callbacks_->onCreateSessionSuccess(obj);
	else 
		callbacks_->onError(rtc::JsonValueToString(obj));
}

AttachPluginTransaction::AttachPluginTransaction(
		PluginType type,	
		IAttachPluginCallbacks* callbacks) 
	: type_(type),
		callbacks_(callbacks) {}

AttachPluginTransaction::~AttachPluginTransaction()
{
	callbacks_ = NULL;
}

void AttachPluginTransaction::reportSuccess(const Json::Value& obj)
{
	//RTC_LOG(INFO) << __FUNCTION__;
	std::string gateway_;
	if (!rtc::GetStringFromJsonObject(obj, "gateway",&gateway_)) {
		//RTC_LOG(WARNING) << "Can't parse received message.";		
		callbacks_->onError("Can't parse received message.");
		return;
	}
	GatewayType type = gatewayTypeFromString(gateway_.c_str());
	if (type != GATEWAY_SUCCESS) 
		callbacks_->onError(rtc::JsonValueToString(obj));
  else 
    callbacks_->attachPluginSuccess(type_,obj);
}

AskTransaction::AskTransaction(
	PluginType type,askInfo* info,IAskCallbacks* callbacks) 
	: type_(type),info_(std::move(info)),callbacks_(callbacks) {}

AskTransaction::~AskTransaction()
{
	callbacks_ = NULL;
	if (info_ != NULL) {
		delete info_;
		info_ = NULL;
	}
}

void AskTransaction::reportSuccess(const Json::Value& obj)
{
	std::string gateway_;
	if (!rtc::GetStringFromJsonObject(obj, "gateway",&gateway_)) {		
		callbacks_->onError("Can't parse received message.");
		return;
	}
	
	GatewayType type = gatewayTypeFromString(gateway_.c_str());
	if (type != GATEWAY_SUCCESS)
		callbacks_->onError(rtc::JsonValueToString(obj));
  else
   	callbacks_->onAskSuccess(type_,info_,obj);
}

std::unique_ptr<ITransaction> TransactionFactory::createNewTransaction(
		TransactionType type,
		ICreateSessionCallbacks* callbacks)
{
	switch (type) {
		case TRANSACTION_CREATE:
			return absl::make_unique<CreateSessionTransaction>(callbacks);
		default:
			return nullptr;	
	}		
}

std::unique_ptr<ITransaction> 
	TransactionFactory::createAskTransaction(
		PluginType pluginType,
		askInfo* info,
		IAskCallbacks* callbacks)
{
	return absl::make_unique<AskTransaction>(pluginType,info,callbacks);
}

#if 1
std::unique_ptr<ITransaction> 
	TransactionFactory::createAttachTransaction(
		PluginType pluginType,
		IAttachPluginCallbacks* callbacks)
{
	return absl::make_unique<AttachPluginTransaction>(
								pluginType,callbacks);
}
#else
std::unique_ptr<ITransaction> TransactionFactory::createNewTransaction(
		TransactionType type,
		PluginType pluginType,
		std::function<void(PluginType type,const Json::Value& obj)> func)
{
	switch (type) {
		case TRANSACTION_ATTACH:
			return absl::make_unique<AttachPluginTransaction>(
										pluginType,std::move(func));
		default:
			return nullptr;	
	} 
}
#endif
