// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-01-04
// Description: 房间模块

#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/lua/export.h>
#include <squick/plugin/net/export.h>

// #include "../client/i_db_module.h"

// #include "../play/i_gameplay_manager_module.h"
// #include "i_player_manager_module.h"

#include <node/game/node/i_node_module.h>

#include "i_room_module.h"
#include <struct/struct.h>
#include <unordered_map>

namespace game::room {

class RoomModule : public IRoomModule {
  public:
    RoomModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }

    virtual bool Start();
    virtual bool Destory();

    virtual bool AfterStart();
    virtual bool Update();

    virtual bool RoomQuit(const Guid &player) override;
    virtual Room *GetRoomByID(int room_id) override;
    virtual void BroadcastToPlayers(const uint16_t msg_id, google::protobuf::Message &xMsg, int roomdID) override;
    virtual void GamePlayPrepared(int room_id, const string &name, const string &ip, int port) override;

  private:
    inline void SendToPlayer(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &player);

  protected:
    void OnReqRoomCreate(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqRoomJoin(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqRoomList(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqRoomDetails(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqRoomQuit(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqRoomPlayerEvent(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqRoomGamePlayStart(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    // Game Play 接口
    void OnReqGameplayData(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqGameplayPrepared(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqGameplayOver(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void CreateDevRoom();

    Room *GetRoom(int room_id);
    inline rpc::RoomDetails GetRoomDetails(int room_id);

    std::unordered_map<int, Room *> rooms_;

  private:
    INetModule *m_net_;
    ILuaScriptModule *m_lua_script_;
    ILogModule *m_log_;
    node::INodeModule *m_node_;
    // IPlayerManagerModule *m_player_manager_;
    // play::IGameplayManagerModule *m_gameplay_manager_;
};

} // namespace game::lobby
