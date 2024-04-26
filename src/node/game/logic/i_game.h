// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-01-04
// Description: 游戏逻辑

#pragma once

#include "i_game_mgr_module.h"
#include <squick/core/base.h>

namespace game::logic {
using namespace rpc;
class IGame {
  public:
    enum Status {
        CREATING,
        CREATED,
        STOPED,
        RUNNING,
        GAMEOVER,
        DESTROYING,
        DESTROYED,
    };

    IGame() { status = CREATED; }

    // 析构函数采用虚函数，不然子类没法正确释放内存
    virtual ~IGame() { status = DESTROYED; }

    virtual void PlayerJoin(const Guid &player) = 0;
    virtual void PlayerQuit(const Guid &player) = 0;
    virtual void AllPlayerJoined() = 0;
    virtual void Awake() = 0;
    virtual void Start() = 0;
    virtual void Update() = 0;
    virtual void Destroy() = 0;

    inline Status GetStatus() { return status; }

    inline void SetStatus(Status status) { this->status = status; }

    inline void DoInit(int id, IGameMgrModule *playManager) {
        this->manager = playManager;
        this->id = id;
        status = STOPED;
    }

    inline void DoAwake() {
        dout << "Game Play Parent Awake!\n";
        MsgBind(GameBaseRPC::REQ_GAME_JOIN, this, &IGame::OnReqGameJoin);
        Awake();
    };

    inline void DoStart() {
        status = RUNNING;
        dout << "Game Play Parent Start!\n";
        // 初始化完毕后，通过告诉房间实现加入
        // manager->m_room_->GamePlayPrepared(id, "game", "", 0);
        Start();
    };

    inline void DoUpdate() { Update(); };

    inline void DoDestroy() {
        status = DESTROYING;
        Destroy();
    };

    void OnReqGameJoin(const Guid &clientID, const int msg_id, const std::string &data) {
        dout << "Req Game Join\n";
        DoPlayerJoin(clientID);
    }

    // 玩家加入
    void DoPlayerJoin(const Guid &player) {

        // int roomd_id = manager->m_player_manager_->GetPlayerRoomID(player);
        // if (id != roomd_id) {
        //     dout << "Join Failed!\n";
        //     return;
        // }

        // 判断是否重复加入
        // if (-1 != manager->m_player_manager_->GetPlayerGameplayID(player)) {
        //    dout << "重复加入!\n";
        //    return;
        //}

        // 判断玩家数量是否超限
        if (onlinePlayerCount > 5) {
            dout << "重复加入!\n";
            return;
        }

        // auto p = manager->m_player_manager_->GetPlayer(player);

        // if (p == nullptr) {
        //     dout << "No this player\n";
        return;
        //}

        BasePlayer pd;
        pd.enterTime = SquickGetTimeMS();
        pd.index = index_id;
        pd.guid = player;
        pd.isOnline = true;

        // pd.name = p->name_;
        // pd.account = p->account_;
        // pd.mask = p->mask_;
        // pd.glove = p->glove_;

        index_id++;
        onlinePlayerCount++;

        base_players[player] = pd;

        // manager->m_player_manager_->SetPlayerGameplayID(player, id);

        rpc::AckGameJoin ack;
        ack.set_code(0);
        SendToPlayer(rpc::GameBaseRPC::ACK_GAME_JOIN, ack, player);

        // 发送玩家列表
        rpc::AckPlayerEnterList xPlayerEntryInfoList;
        for (auto &iter : base_players) {
            auto &player = iter.second;
            rpc::PlayerBaseInfo *pEntryInfo = xPlayerEntryInfoList.add_list();
            *(pEntryInfo->mutable_guid()) = player.guid.ToString();
            pEntryInfo->set_index(player.index);
            pEntryInfo->set_name(player.name);
            pEntryInfo->set_account(player.account);
        }

        // 发送当前所有玩家给加入者
        SendToPlayer(GameBaseRPC::ACK_GAME_PLAYER_ENTER, xPlayerEntryInfoList, player);

        // 发送新玩家给其他已加入的玩家
        AckPlayerEnterList xNewPlayerEntryInfoList;
        PlayerBaseInfo *pEntryInfo = xNewPlayerEntryInfoList.add_list();
        *(pEntryInfo->mutable_guid()) = player.ToString();
        pEntryInfo->set_index(pd.index);
        // pEntryInfo->set_name(p->name_);
        // pEntryInfo->set_account(p->account_);
        // pEntryInfo->set_mask(p->mask_);
        // pEntryInfo->set_glove(p->glove_);

        // 广播新加入者
        BroadcastToPlayersExcept(GameBaseRPC::ACK_GAME_PLAYER_ENTER, xNewPlayerEntryInfoList, player);

        // 调用子类
        PlayerJoin(player);
        // if (base_players.size() == manager->m_room_->GetRoomByID(id)->players.size()) {
        //    AllPlayerJoined();
        //}
    }

    void DoPlayerQuit(const Guid &p) {
        auto iter = base_players.find(p);
        if (iter == base_players.end()) {
            dout << "Error not found this player\n";
            return;
        }
        auto &player = iter->second;
        player.isOnline = false;
        onlinePlayerCount--;
        dout << "Online Count: " << onlinePlayerCount << std::endl;
        PlayerQuit(player.guid);
    }

    void SendToPlayer(int msg_id, google::protobuf::Message &xMsg, const Guid &player) {
        // manager->m_node_->SendPBToPlayer(msg_id, xMsg, player);
    }

    void BroadcastToPlayers(int msg_id, google::protobuf::Message &xMsg) {
        for (auto const &iter : base_players) {
            auto &player = iter.second;
            if (player.isOnline == true) {

                // manager->m_node_->SendPBToPlayer(msg_id, xMsg, player.guid);
            }
        }
    }

    void BroadcastToPlayersExcept(int msg_id, google::protobuf::Message &xMsg, const Guid &exceptPlayer) {
        for (auto const &iter : base_players) {
            auto &player = iter.second;
            if (player.guid == exceptPlayer) {
                // dout << "BroadcastToPlyaersExcept: Except player:  " << exceptPlayer.ToString() << std::endl;
                continue;
            }
            if (player.isOnline == true) {
                // manager->m_node_->SendPBToPlayer(msg_id, xMsg, player.guid);
            }
        }
    }

    void BroadcastToActivePlayers(int msg_id, google::protobuf::Message &xMsg) {
        for (auto const &iter : base_players) {
            auto &player = iter.second;
            if (player.isOnline && player.isActive) {

                // manager->m_node_->SendPBToPlayer(msg_id, xMsg, player.guid);
            }
        }
    }

    void BroadcastToActivePlayersExcept(int msg_id, google::protobuf::Message &xMsg, const Guid &exceptPlayer) {
        for (auto const &iter : base_players) {
            auto &player = iter.second;
            if (player.guid == exceptPlayer) {
                continue;
            }
            if (player.isOnline && player.isActive) {
                // manager->m_node_->SendPBToPlayer(msg_id, xMsg, player.guid);
            }
        }
    }

    inline int OnlinePlayerCount() { return onlinePlayerCount; }

    template <typename BaseType>
    bool MsgBind(const int msg_id, BaseType *pBase, void (BaseType::*handleReceiver)(const Guid &clientID, const int msg_id, const std::string &data)) {
        return manager->AddReceiveCallBack(msg_id, id, pBase, handleReceiver);
    }

    inline bool CheckIsHaveThisPlayer(const Guid &p) {
        for (auto &iter : base_players) {
            auto &player = iter.second;
            if (player.guid == p) {
                return true;
            }
        }
        return false;
    }

    inline void SetPlayerActive(const Guid &player, bool isActive) {
        auto iter = base_players.find(player);
        if (iter == base_players.end()) {
            dout << "Not found this player\n";
            return;
        }
        iter->second.isActive = isActive;
    }

    struct BasePlayer {
        int index = 0;
        Guid guid;
        time_t enterTime = 0;
        bool isOnline = true;
        bool isActive = true;
        string name;
        string account;
    };

    int GetID() { return id; }

  private:
    int onlinePlayerCount = 0;
    IGameMgrModule *manager = nullptr;
    int id = 0;
    Status status = Status::CREATING;
    map<Guid, BasePlayer> base_players;
    int index_id = 0;
};
} // namespace game::logic