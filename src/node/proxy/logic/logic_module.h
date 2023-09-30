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
    void OnReqEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    bool TryEnter(string guid);
    int EnterSuccessEvent(const string account_id, const string player_id);
    bool SendToPlayer(string player_id, const int msg_id, const string& data);
    virtual void OnClientDisconnected(socket_t sock) override;

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
        string account_id;
        time_t last_ping = 0;   // ms time
        socket_t sock = -1;
        string ip = "";
        Status status = PlayerOffline;
        string player_id;
        int world_id = 0;
        int lobby_id = 0;
        int micro_id = 0;
        int game_id = 0;
    };

    unordered_map<string, KeepAlive> clients_; // key: account_id , value: info
    unordered_map<string, string> players_;    // key: palyer_id, value: account_id
    struct Session {
        socket_t sock;
        time_t time;
        string account_id;
        string key;
        string ip;
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