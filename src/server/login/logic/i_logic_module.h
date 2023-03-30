#pragma once

#include <squick/plugin/net/i_net_module.h>

namespace login::logic {

class ILogicModule : public IModule {
  public:
    virtual void OnLoginProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) = 0;

    /*
    virtual bool LoginWithAccountPasswrod() = 0;
    virtual bool LoginWithPhonePasswrod() = 0;
    virtual bool LoginWithEmailPassword() = 0;
    virtual bool LoginWithEmailVerifyCode() = 0;
    virtual bool LoginWithPhoneVerifyCode() = 0;
    virtual bool LoginWithWechat() = 0;
    virtual bool LoginWithQQ() = 0;*/
};

} // namespace login::logic