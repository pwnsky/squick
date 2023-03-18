// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-01-04
// Description: 游戏逻辑

#pragma once

#include "i_gameplay_manager_module.h"
#include <squick/core/base.h>

namespace game::play {
using namespace SquickStruct;
class IGameplay {
  public:
    enum Status {
        CREATING,
        CREATED,
        STOPED,
        RUNNING,
        DESTROYING,
        DESTROYED,
    };
    IGameplay() { status = CREATED; }

    // 析构函数采用虚函数，不然子类没法正确释放内存
    virtual ~IGameplay() { status = DESTROYED; }

    virtual void PlayerJoin(const Guid &player) = 0;
    virtual void PlayerQuit(const Guid &player) = 0;
    virtual void AllPlayerJoined() = 0;
    virtual void Awake() = 0;
    virtual void Start() = 0;
    virtual void Update() = 0;
    virtual void Destroy() = 0;

    inline Status GetStatus() { return status; }

    inline void DoInit(int id, IGameplayManagerModule *playManager) {
        this->manager = playManager;
        this->id = id;
        status = STOPED;
    }

    inline void DoAwake() {
        dout << "Game Play Parent Awake!\n";
        MsgBind(GameBaseRPC::REQ_GAME_JOIN, this, &IGameplay::OnReqGameJoin);
        Awake();
    };

    inline void DoStart() {
        status = RUNNING;
        dout << "Game Play Parent Start!\n";
        // 初始化完毕后，通过告诉房间实现加入
        manager->m_pRoomModule->GamePlayPrepared(id, "game", "", 0);
        Start();
    };

    inline void DoUpdate() { Update(); };

    inline void DoDestroy() {
        status = DESTROYING;
        Destroy();
    };

    void OnReqGameJoin(const Guid &clientID, const int msgID, const std::string &data) {
        dout << "Req Game Join\n";
        DoPlayerJoin(clientID);
    }

    // 玩家加入
    void DoPlayerJoin(const Guid &player) {

        int roomd_id = manager->m_pPlayerManagerModule->GetPlayerRoomID(player);
        if (id != roomd_id) {
            dout << "Join Failed!\n";
            return;
        }

        // 判断是否重复加入
        if (-1 != manager->m_pPlayerManagerModule->GetPlayerGameplayID(player)) {
            dout << "重复加入!\n";
            return;
        }

        // 判断玩家数量是否超限
        if (onlinePlayerCount > 5) {
            dout << "重复加入!\n";
            return;
        }

        BasePlayer pd;
        pd.enterTime = SquickGetTimeMS();
        pd.index = index_id;
        pd.guid = player;
        pd.isOnline = true;
        index_id++;
        onlinePlayerCount++;
        base_players.push_back(pd);

        manager->m_pPlayerManagerModule->SetPlayerGameplayID(player, id);

        SquickStruct::AckGameJoin ack;
        ack.set_code(0);
        SendToPlayer(SquickStruct::GameBaseRPC::ACK_GAME_JOIN, ack, player);

        // 发送玩家列表
        SquickStruct::AckPlayerEnterList xPlayerEntryInfoList;
        for (auto &player : base_players) {
            const Guid &identOld = player.guid;
            if (identOld.IsNull()) {
                continue;
            }
            SquickStruct::PlayerBaseInfo *pEntryInfo = xPlayerEntryInfoList.add_list();
            *(pEntryInfo->mutable_guid()) = INetModule::StructToProtobuf(identOld);
            pEntryInfo->set_index(player.index);
        }

        // 发送当前所有玩家给加入者
        SendToPlayer(GameBaseRPC::ACK_PLAYER_ENTER, xPlayerEntryInfoList, player);

        // 发送新玩家给其他已加入的玩家
        AckPlayerEnterList xNewPlayerEntryInfoList;
        PlayerBaseInfo *pEntryInfo = xNewPlayerEntryInfoList.add_list();
        *(pEntryInfo->mutable_guid()) = INetModule::StructToProtobuf(player);
        pEntryInfo->set_index(pd.index);

        // 广播新加入者
        BroadcastToPlyaersExcept(GameBaseRPC::ACK_PLAYER_ENTER, xNewPlayerEntryInfoList, player);

        // 调用子类
        PlayerJoin(player);
        if (base_players.size() == manager->m_pRoomModule->GetRoomByID(id)->players_size()) {
            AllPlayerJoined();
        }
    }

    // 玩家退出
    void DoPlayerQuit(const Guid &player) {
        for (auto iter = base_players.begin(); iter < base_players.end(); ++iter) {
            if (iter->guid == player) {
                // 暂时不删除
                iter->isOnline = false;
                onlinePlayerCount--;
                dout << "Online Count: " << onlinePlayerCount << std::endl;
                PlayerQuit(player);
                break;
            }
        }
    }

    void SendToPlayer(int msgID, google::protobuf::Message &xMsg, const Guid &player) {
        // dout << " 发送给客户端: " << player.ToString() << "   MSGID: " << msgID << std::endl;
        manager->m_pGameServerNet_ServerModule->SendMsgPBToGate(msgID, xMsg, player);
    }

    //
    void BroadcastToPlyaers(int msgID, google::protobuf::Message &xMsg) {
        for (auto const &player : base_players) {
            if (player.isOnline == true) {
                // dout << " 广播发送给客户端: " << player.first.ToString() << "   MSGID: " << msgID << std::endl;
                manager->m_pGameServerNet_ServerModule->SendMsgPBToGate(msgID, xMsg, player.guid);
            }
        }
    }

    void BroadcastToPlyaersExcept(int msgID, google::protobuf::Message &xMsg, const Guid &exceptPlayer) {
        for (auto const &player : base_players) {
            if (player.guid == exceptPlayer) {
                // dout << "BroadcastToPlyaersExcept: Except player:  " << exceptPlayer.ToString() << std::endl;
                continue;
            }
            if (player.isOnline == true) {
                // dout << " 广播发送给客户端: " << player.first.ToString() << "   MSGID: " << msgID << std::endl;
                manager->m_pGameServerNet_ServerModule->SendMsgPBToGate(msgID, xMsg, player.guid);
            }
        }
    }
    inline int OnlinePlayerCount() { return onlinePlayerCount; }

    template <typename BaseType>
    bool MsgBind(const int msgID, BaseType *pBase, void (BaseType::*handleReceiver)(const Guid &clientID, const int msgID, const std::string &data)) {
        return manager->AddReceiveCallBack(msgID, id, pBase, handleReceiver);
    }

    inline bool CheckIsHaveThisPlayer(const Guid &player) {
        for (auto &p : base_players) {
            if (p.guid == player) {
                return true;
            }
        }
        return false;
    }

    struct BasePlayer {
        int index = 0;
        Guid guid;
        time_t enterTime = 0;
        bool isOnline = true;
    };

    int GetID() { return id; }

  private:
    int onlinePlayerCount = 0;
    IGameplayManagerModule *manager = nullptr;
    int id = 0;
    Status status = Status::CREATING;
    std::vector<BasePlayer> base_players;
    int index_id = 0;
};
} // namespace game::play