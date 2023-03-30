#pragma once

#include <squick/core/consistent_hash.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/kernel/i_thread_pool_module.h>
#include <squick/struct/struct.h>

#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/plugin/net/i_ws_module.h>
#include <squick/plugin/security/i_security_module.h>

#include "../client/i_game_module.h"
#include "../client/i_world_module.h"
#include "i_server_module.h"
#include <server/proxy/logic/i_logic_module.h>

namespace proxy::server {
class ServerModule : public IServerModule {
  public:
    ServerModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

  protected:
    void OnSocketClientEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);
  protected:
    INetClientModule *m_net_client_;
    IKernelModule *m_kernel_;
    ILogModule *m_log_;
    IElementModule *m_element_;
    IClassModule *m_class_;
    INetModule *m_net_;
    ISecurityModule *m_security_;
    client::IWorldModule *m_pProxyToWorldModule;
    logic::ILogicModule* m_logic_;
    IThreadPoolModule *m_thread_pool_;
};

} // namespace proxy::server