#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/utils/export.h>

#include "i_logic_module.h"
#include <node/proxy/node/i_node_module.h>
#include <vector>
enum class ProtocolType {
    Tcp,
    WS,
};

enum Status {
    PlayerOffline,
    PlayerOnline,
    PlayerNotEneter,
    PlayerHeatbeatTimeout,
};

struct PlayerConnInfo {
    ProtocolType protocol_type = ProtocolType::Tcp;
    string account;
    string account_id;
    time_t last_ping = 0;   // ms time
    socket_t sock = -1;
    string ip = "";
    Status status = PlayerOffline;
    uint64_t uid;
    int player_node = 0;
    int world_node = 0;
};

struct Session {
    ProtocolType protocol_type = ProtocolType::Tcp;
    socket_t sock;
    time_t time;
    string account_id;
    string key;
    string ip;
};

namespace proxy::logic {
class LogicModule : public ILogicModule {
  public:


    LogicModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start() override;
    virtual bool Destory() override;
    virtual bool Update() override;
    virtual bool AfterStart() override;

  protected:
    void OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnHeartbeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqTestProxy(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);

    Coroutine<bool> OnReqConnectWithTcp(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqConnectWithWS(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqConnect(ProtocolType type, const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnNAckConnectVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    
    void OnReqPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnAckPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqPlayerLeave(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    
    void NReqMinWorkloadNodeInfo();
    void OnNAckMinWorkloadNodeInfo(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);

    bool SendToPlayer(uint64_t uid, const int msg_id, const string& data);
    virtual void OnClientDisconnected(socket_t sock) override;

    void OnRecivedPlayerNodeMsg(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    int OnHeatbeatCheck(const Guid &self, const std::string &heartBeat, const float time, const int count);

    void OnWS(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnWebSocketClientEvent(socket_t sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet);
    
    int GetLoadBanlanceNode(ServerType type);
    PlayerConnInfo* GetPlayerConnInfo(const uint64_t uid);
    PlayerConnInfo* GetPlayerConnInfo(const socket_t player_sock);
    bool RemovePlayerConnInfo(const socket_t player_sock);

    unordered_map<socket_t, PlayerConnInfo> players_;    // key: sock , value: info
    unordered_map<uint64_t, socket_t> players_socks_;    // key: uid, value: sock

    map<socket_t, Session> sessions_;
    map<int, int> min_workload_nodes_; // min workload nodes, key: node type, value: node id

  protected:
    ILogModule *m_log_;
    IClassModule *m_class_;
    IScheduleModule *m_schedule_;
    INetModule *m_net_;
    IWSModule* m_ws_;
    INetClientModule *m_net_client_;
    node::INodeModule *m_node_;



  private:
    time_t last_update_work_load_info_time_ = 0;
};

} // namespace proxy::logic
