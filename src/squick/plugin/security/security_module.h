#pragma once

#include <memory>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include "i_security_module.h"

class SecurityModule
    : public ISecurityModule
{
public:
	SecurityModule(IPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

	//when a user login NF system, as a security module, you need to grant user an key.
	virtual const std::string GetSecurityKey(const std::string& account);

	//when the user want to connect to proxy, as a user need to provide the key that got in login system.
	virtual bool VerifySecurityKey(const std::string& account, const std::string& strSecurityKey);

	//when the user want to post a message to proxy, the message-data need to be verified.
	virtual std::string EncodeMsg(const std::string& account, const std::string& strSecurityKey, const int nMessageID, const char* strMessageData, const int len);
	virtual std::string EncodeMsg(const std::string& account, const std::string& strSecurityKey, const int nMessageID, const std::string& strMessageData);
	virtual std::string DecodeMsg(const std::string& account, const std::string& strSecurityKey, const int nMessageID, const char* strMessageData, const int len);
	virtual std::string DecodeMsg(const std::string& account, const std::string& strSecurityKey, const int nMessageID, const std::string& strMessageData);

protected:

    //////////////////////////////////////////////////////////////////////////
    IKernelModule* m_pKernelModule;
    IClassModule* m_pClassModule;
	ILogModule* m_pLogModule;
	IElementModule* m_pElementModule;
    //////////////////////////////////////////////////////////////////////////
};
