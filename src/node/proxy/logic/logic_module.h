#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_logic_module.h"
#include <node/proxy/node/i_node_module.h>
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
    void OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    
    void OnHeartbeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqTestProxy(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);

    void OnReqConnect(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    virtual void OnAckConnectVerify(const int msg_id, const char *msg, const uint32_t len) override;
    void OnReqEnterGameServer(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    bool SelectGameServer(int sock);
    int EnterGameSuccessEvent(const Guid xClientID, const Guid xPlayerID);

    

    virtual void OnClientConnected(const socket_t sock) override;
    virtual void OnClientDisconnect(const socket_t sock) override;

    int ForwardToClient(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    int OnHeatbeatCheck(const Guid &self, const std::string &heartBeat, const float time, const int count);

    // MapEx<Guid, socket_t> mxClientIdent; // player ident
    enum Status {
        PlayerOffline,
        PlayerOnline,
        PlayerNotEneter,
        PlayerHeatbeatTimeout,
    };

    struct KeepAlive {
        string account;
        Guid guid = Guid(0, 0); // object guid
        time_t last_ping = 0;   // ms time
        socket_t sock = -1;

        Status status = PlayerOffline;

        int world_id = 0;
        int lobby_id = 0;
        int micro_id = 0;
        int game_id = 0;
        int gameplay_manager_id = 0;
    };

    unordered_map<string, KeepAlive> clients_; // string is guid.ToString

    struct Session {
        socket_t sock;
        time_t time;
        Guid guid;
        string key;
    };

    map<INT64, Session> sessions_;

  protected:
    ILogModule *m_log_;
    IClassModule *m_class_;
    IKernelModule *m_kernel_;
    IScheduleModule *m_schedule_;
    INetModule *m_net_;
    INetClientModule *m_net_client_;
    node::INodeModule *m_node_;

  private:
};

} // namespace proxy::logic