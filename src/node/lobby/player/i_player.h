// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-04-10
// Description: 玩家进入游戏后，由玩家管理器创建出来的对象

#pragma once
#include "i_player_manager_module.h"
#include <google/protobuf/message.h>
#include <squick/core/base.h>
#include <squick/core/guid.h>
#include <string>

namespace lobby::player {
using namespace std;

class IPlayer {
  public:
    IPlayer() {
        // dout << "玩家进入游戏\n";
    }

    virtual ~IPlayer() {
        // dout << "玩家对象销毁\n";
    }

    // 刚进入游戏
    void OnEnterGame() {}

    // 离线退出
    void OnOffline() {}

    // 即将销毁
    void OnDestroy() {}

    template <typename BaseType>
    bool MsgBind(const int msg_id, BaseType *pBase, void (BaseType::*handleReceiver)(const Guid &clientID, const int msg_id, const std::string &data)) {
        return manager->AddReceiveCallBack(msg_id, pBase, handleReceiver);
    }

    inline int GetRoomID() { return room_id_; }

    inline void SetRoomID(int id) { this->room_id_ = id; }

    inline void SetGameplayID(int id) { this->gameplay_id_ = id; }

    inline int GetGameplayID() { return this->gameplay_id_; }

    void SendToPlayer(int msg_id, google::protobuf::Message &xMsg, const Guid &player) {
        // dout << " 发送给客户端: " << player.ToString() << "   MSGID: " << msg_id << std::endl;
        manager->m_node_->SendPBToPlayer(msg_id, xMsg, player);
    }

    int offline_time_ = 0;
    int login_time_ = 0;
    string account_;
    string name_;
    Guid object_;
    Guid guid_;
    IPlayerManagerModule *manager = nullptr;

  private:
    int room_id_ = -1;
    int gameplay_id_ = -1;
};

} // namespace lobby::player