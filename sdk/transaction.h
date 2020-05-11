#ifndef GATEWAY_CLIENT_TRANSACTION_H
#define GATEWAY_CLIENT_TRANSACTION_H
#include "rtc_base/strings/json.h"
#include "rtc_base/logging.h"
#include "plugin_handle.h"
#include "helper.h"
#include <functional>

enum TransactionType {
  TRANSACTION_CREATE,
  TRANSACTION_ATTACH,
  TRANSACTION_ASK,
  TRANSACTION_MESSAGE,
  TRANSACTION_TRICKLE,
  TRANSACTION_PLUGIN_MESSAGE,
  TRANSACTION_PLUGIN_WEBRTC_MESSAGE
};

class askInfo {
 public:	
	std::string peerName;
	std::string sdp;
	std::string type;
};

class IErrorCallbacks {
 public:	
  virtual ~IErrorCallbacks() = default;	
	virtual void onError (const std::string& error) = 0;
};

class ICreateSessionCallbacks : public IErrorCallbacks {
 public:	
  virtual ~ICreateSessionCallbacks() = default;
  virtual void onCreateSessionSuccess(const Json::Value& obj) = 0;
};

class IAttachPluginCallbacks : public IErrorCallbacks  {
public:
  virtual ~IAttachPluginCallbacks() = default;
	virtual void attachPluginSuccess(
					PluginType type,
	        const Json::Value& obj) = 0;
};

class IAskCallbacks : public IErrorCallbacks  {
public:
  virtual ~IAskCallbacks() = default;
	virtual void onAskSuccess(
					PluginType type,
					askInfo* info,
					const Json::Value& obj) = 0;
};

class ITransaction {
 public:	
  virtual ~ITransaction() = default;
  
  virtual void reportSuccess(const Json::Value& obj) = 0;
  
  virtual TransactionType getType() = 0;
};

class CreateSessionTransaction : public ITransaction {
 public: 
  explicit CreateSessionTransaction(
		ICreateSessionCallbacks* callbacks);
	
  virtual ~CreateSessionTransaction();	
  
  void reportSuccess(const Json::Value& obj) override;
  TransactionType getType() override {return TRANSACTION_CREATE;}; 
	
 private:
  ICreateSessionCallbacks* callbacks_;
};


class AskTransaction : public ITransaction {
 public: 
  explicit AskTransaction(
		PluginType type,askInfo* info,IAskCallbacks* callbacks);
	
  virtual ~AskTransaction();	
  
  void reportSuccess(const Json::Value& obj) override;
	
  TransactionType getType() override {return TRANSACTION_ASK;}; 
	
 private: 	
 	PluginType type_;
	askInfo* info_;
	IAskCallbacks* callbacks_; 	
};

#if 1
class AttachPluginTransaction : public ITransaction {
 public: 
  explicit AttachPluginTransaction(	
		PluginType type,	
		IAttachPluginCallbacks* callbacks);
	
  virtual ~AttachPluginTransaction();	
  
  void reportSuccess(const Json::Value& obj) override;
	
  TransactionType getType() override {return TRANSACTION_ATTACH;}; 
	
 private: 	
 	PluginType type_;
	IAttachPluginCallbacks* callbacks_; 	
};
#else
class AttachPluginTransaction : public ITransaction {
 public: 
  explicit AttachPluginTransaction(	
		PluginType type,
		std::function<void(
			PluginType type,
			const Json::Value& obj)> func) 
				: type_(type),func_(std::move(func)) {}
	
  virtual ~AttachPluginTransaction() = default;	
  
  void reportSuccess(const Json::Value& obj) override;
	
  TransactionType getType() override {return TRANSACTION_ATTACH;}; 
	
 private: 
 	PluginType type_;
	std::function<void(
		PluginType type,
		const Json::Value& obj)> func_;
};

#endif
class TransactionFactory {
 public:	
  static std::unique_ptr<ITransaction>
    createNewTransaction(TransactionType type,
                         ICreateSessionCallbacks* callbacks);

  static std::unique_ptr<ITransaction>
    createAttachTransaction(
    										 PluginType pluginType,
				                 IAttachPluginCallbacks* callbacks);	

  static std::unique_ptr<ITransaction>
    createAskTransaction(PluginType pluginType,
    										 askInfo* info,
				                 IAskCallbacks* callbacks);		
#if 0		

  static std::unique_ptr<ITransaction>
    createNewTransaction(TransactionType type,
    										 PluginType pluginType,
				                 std::function<void(PluginType type,const Json::Value& obj)> func);	

  static std::unique_ptr<ITransaction>
    createNewTransaction(
								                 TransactionType type,
                                 /*SupportedPluginPackages> plugin,*/
                                 IPluginHandleWebRTCCallbacks* callbacks);
	
  static std::unique_ptr<ITransaction>
    createNewTransaction(
								                 TransactionType type,
                                 /*SupportedPluginPackages plugin,*/
                                 IPluginHandleSendMessageCallbacks* callbacks);	

#endif	
};
#endif
