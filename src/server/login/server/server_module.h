#pragma once
#include "squick/core/map.h"

#include <squick/core/base.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "../client/i_master_module.h"
#include "../logic/i_logic_module.h"
#include "i_server_module.h"

#include "squick/struct/struct.h"

namespace login::server {
class ServerModule : public IServerModule {
    
  public:
    ServerModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool BeforeDestory();
    virtual bool AfterStart();

    virtual void LogReceive(const char *str) {}
    virtual void LogSend(const char *str) {}

  protected:
    void OnSocketClientEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

  protected:
    void OnClientDisconnect(const socket_t sock);
    void OnClientConnected(const socket_t sock);
    void OnSelectWorldProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  protected:
  protected:
    void SynWorldToClient(const socket_t nFD);

    MapEx<Guid, socket_t> mxClientIdent;

  private:
    INetModule *m_net_;
    INetClientModule *m_net_client_;
    IClassModule *m_class_;
    IElementModule *m_element_;
    IKernelModule *m_kernel_;
    ILogModule *m_log_;
    client::IMasterModule *m_pLoginToMasterModule;
    IThreadPoolModule *m_thread_pool_;
};

} // namespace login::server