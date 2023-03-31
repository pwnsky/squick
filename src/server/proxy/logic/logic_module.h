#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_logic_module.h"
#include <vector>

namespace proxy::logic {
class LogicModule : public ILogicModule {
  public:
    LogicModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

  protected:
    void OnOtherMessage(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnHeartbeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqConnect(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqEnterGameServer(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    bool SelectGameServer(int sock);
    int EnterGameSuccessEvent(const Guid xClientID, const Guid xPlayerID);

    virtual void OnClientConnected(const socket_t sock) override;
    virtual void OnClientDisconnect(const socket_t sock) override;
    
    int ForwardToClient(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    bool RemovePlayer(const Guid &xPlayerID);

    MapEx<Guid, socket_t> mxClientIdent; // player ident
    
    struct KeepAlive {
        socket_t sock = -1;
        time_t last_ping = 0; // ms time
    };

  protected:
    ILogModule* m_log_;
    IClassModule *m_class_;
    IKernelModule *m_kernel_;
    INetModule *m_net_;
    INetClientModule *m_net_client_;

  private:
};

}