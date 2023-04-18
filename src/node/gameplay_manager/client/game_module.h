#pragma once

#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>
#include <string>

#include <squick/plugin/net/i_net_client_module.h>

#include "../logic/i_logic_module.h"
#include "../server/i_server_module.h"

namespace gameplay_manager::client {
class IGameModule : public IModule {
  public:
};

class GameModule : public IGameModule {
  public:
    GameModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

  protected:
    void Transport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    ILogModule *m_log_;
    IKernelModule *m_kernel_;
    server::IServerModule *m_ServerModule;
    IElementModule *m_element_;
    IClassModule *m_class_;
    INetClientModule *m_net_client_;
};

} // namespace gameplay_manager::client