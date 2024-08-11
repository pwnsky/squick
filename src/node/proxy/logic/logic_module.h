#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/node/export.h>
#include <squick/plugin/utils/export.h>

#include "i_logic_module.h"
#include <vector>

namespace proxy::logic {
#define HEATBEAT_TIMEOUT 30 // seconds
enum class ProtocolType {
    Tcp = 1,
    WS = 2,
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
    time_t last_ping = 0; // ms time
    socket_t sock = -1;
    string ip = "";
    Status status = PlayerOffline;
    uint64_t uid;
    int player_node = 0;
    int world_node = 0;
    int login_node = 0;
};

struct Session {
    ProtocolType protocol_type = ProtocolType::Tcp;
    socket_t sock;
    time_t time;
    string account_id;
    string key;
    string ip;
    int login_node;
};

class LogicModule : public ILogicModule {
  public:
    LogicModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = false;
    }

    virtual bool Start() override;
    virtual bool Destroy() override;
    virtual bool Update() override;
    virtual bool AfterStart() override;

  protected:
    void OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnHeartbeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqTestProxy(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    Coroutine<bool> OnReqConnectWithTcp(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqConnectWithWS(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqConnect(ProtocolType type, const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNAckConnectVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnReqPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnAckPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqPlayerLeave(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    bool SendToPlayer(uint64_t uid, const int msg_id, const string &data);
    virtual void OnClientDisconnected(socket_t sock) override;

    void OnRecivedPlayerNodeMsg(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    int OnHeatbeatCheck(const Guid &self, const std::string &heartBeat, const float time, const int count);

    void OnWS(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnSocketEvent(socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

    int GetLoadBanlanceNode(int type);
    PlayerConnInfo *GetPlayerConnInfoByUID(const uint64_t uid);
    PlayerConnInfo *GetPlayerConnInfo(const socket_t player_sock);
    bool RemovePlayerConnInfo(const socket_t player_sock);

    map<socket_t, PlayerConnInfo> players_; // key: sock , value: info
    map<uint64_t, socket_t> players_socks_; // key: uid, value: sock

    map<socket_t, Session> sessions_;

  protected:
    ILogModule *m_log_;
    IScheduleModule *m_schedule_;
    INetModule *m_net_;
    IWSModule *m_ws_;
    INetClientModule *m_net_client_;
    INodeModule *m_node_;

  private:
    time_t last_update_work_load_info_time_ = 0;
};

} // namespace proxy::logic
