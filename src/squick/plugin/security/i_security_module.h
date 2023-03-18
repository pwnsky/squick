#pragma once

#include <squick/core/i_module.h>

class ISecurityModule : public IModule {
  public:
    // when a user login NF system, as a security module, you need to grant user an key.
    virtual const std::string GetSecurityKey(const std::string &account) = 0;

    // when the user want to connect to proxy, as a user need to provide the key that got in login system.
    virtual bool VerifySecurityKey(const std::string &account, const std::string &strSecurityKey) = 0;

    // when the user want to post a message to proxy, the message-data need to be verified.
    virtual std::string EncodeMsg(const std::string &account, const std::string &strSecurityKey, const int nMessageID, const char *strMessageData,
                                  const int len) = 0;
    virtual std::string EncodeMsg(const std::string &account, const std::string &strSecurityKey, const int nMessageID, const std::string &strMessageData) = 0;
    virtual std::string DecodeMsg(const std::string &account, const std::string &strSecurityKey, const int nMessageID, const char *strMessageData,
                                  const int len) = 0;
    virtual std::string DecodeMsg(const std::string &account, const std::string &strSecurityKey, const int nMessageID, const std::string &strMessageData) = 0;
};
