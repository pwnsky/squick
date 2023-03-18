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

namespace proxy::server {
class ServerModule : public IServerModule {
  public:
    ServerModule(IPluginManager *p) { pPluginManager = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();
    virtual int Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    virtual int EnterGameSuccessEvent(const Guid xClientID, const Guid xPlayerID);

  protected:
    void OnSocketClientEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet);
    void OnClientDisconnect(const SQUICK_SOCKET nAddress);
    void OnClientConnected(const SQUICK_SOCKET nAddress);
    void OnReqLogin(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqEnterGameServer(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    void OnOtherMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

  protected:
    MapEx<Guid, SQUICK_SOCKET> mxClientIdent;
    bool SelectGameServer(int sockIndex);

  protected:
    INetClientModule *m_pNetClientModule;
    IKernelModule *m_pKernelModule;
    ILogModule *m_pLogModule;
    IElementModule *m_pElementModule;
    IClassModule *m_pClassModule;
    INetModule *m_pNetModule;
    IWSModule *m_pWsModule;
    ISecurityModule *m_pSecurityModule;
    client::IWorldModule *m_pProxyToWorldModule;
    IThreadPoolModule *m_pThreadPoolModule;
};

} // namespace proxy::server