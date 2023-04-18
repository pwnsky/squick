#pragma once

#include <squick/core/consistent_hash.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/net/i_ws_module.h>
#include <squick/plugin/security/i_security_module.h>
#include <squick/struct/struct.h>

#include "../client/i_game_module.h"
#include "../client/i_world_module.h"
#include "i_ws_module.h"

namespace proxy::node {
class ProxyServerNet_WSModule : public IWS_Module {
  public:
    ProxyServerNet_WSModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

  protected:
    void OnSocketClientEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

    void OnClientDisconnect(const socket_t sock);
    void OnClientConnected(const socket_t sock);

    void OnWebSocketTestProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  protected:
    MapEx<Guid, socket_t> mxClientIdent;

  protected:
    INetClientModule *m_net_client_;
    IKernelModule *m_kernel_;
    ILogModule *m_log_;
    IElementModule *m_element_;
    IClassModule *m_class_;
    IWSModule *m_pWSModule;
    ISecurityModule *m_security_;
    client::IWorldModule *m_pProxyToWorldModule;
};

} // namespace proxy::server