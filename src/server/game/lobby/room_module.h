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

#include "../client/i_db_module.h"
#include "../play/i_gameplay_manager_module.h"
#include "../server/i_server_module.h"
#include "i_player_manager_module.h"
#include "i_room_module.h"

namespace game::player {

class RoomModule : public IRoomModule {
  public:
    RoomModule(IPluginManager *p) {
        pPluginManager = p;
        m_bIsUpdate = false;
    }

    virtual bool Start();
    virtual bool Destory();

    virtual bool AfterStart();
    virtual bool Update();

    virtual bool RoomQuit(const Guid &clientID) override;
    virtual SquickStruct::RoomDetails *GetRoomByID(int room_id) override;
    virtual void BroadcastToPlyaers(const uint16_t msgID, google::protobuf::Message &xMsg, int roomdID) override;
    virtual void GamePlayPrepared(int room_id, const string &name, const string &ip, int port) override;

  protected:
    void OnReqRoomCreate(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqRoomJoin(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqRoomList(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqRoomDetails(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqRoomQuit(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqRoomPlayerEvent(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqRoomGamePlayStart(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    // Game Play 接口
    void OnReqGameplayData(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqGameplayPrepared(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqGameplayOver(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    void CreateDevRoom();

    std::map<int, SquickStruct::RoomDetails *> m_rooms;

  private:
    INetModule *m_pNetModule;
    ILuaScriptModule *m_pLuaScriptModule;
    ILogModule *m_pLogModule;
    IGameServerNet_ServerModule *m_pGameServerNet_ServerModule;
    IPlayerManagerModule *m_pPlayerManagerModule;
    play::IGameplayManagerModule *m_pGameplayManagerModule;
};

} // namespace game::player
