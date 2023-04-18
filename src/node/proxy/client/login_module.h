#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <squick/struct/struct.h>

#include "../logic/i_logic_module.h"
#include "../server/i_server_module.h"
#include "i_login_module.h"
#include <server/proxy/logic/i_logic_module.h>

namespace proxy::client {
class LoginModule : public ILoginModule {
  public:
    LoginModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }
    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

  protected:
    virtual bool OnReqProxyConnectVerify(INT64 session, const std::string &guid, const std::string &key) override;
    void OnAckProxyConnectVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    int login_id_ = 0;
    logic::ILogicModule *m_logic_;
};

} // namespace proxy::client