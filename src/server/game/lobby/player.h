
// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-01-05
// Description: 玩家进入游戏后，由玩家管理器创建出来的对象
#include <squick/core/base.h>
#include <squick/core/guid.h>

#pragma once
namespace game::player {

class Player {
  public:
    Player();
    virtual ~Player();
    void OnEnterGame();
    void OnOffline();
    void OnDestroy();
    void OnEnterScene();
    void OnExitScene();
    void OnChangeScene();

    int GetRoomID() { return room_id_; }

    void SetRoomID(int id) { this->room_id_ = id; }

    void SetGameplayID(int id) { this->gameplay_id_ = id; }

    int GetGameplayID() { return this->gameplay_id_; }

    int offlineTime = 0;
    int loginTime = 0;

  private:
    int room_id_ = -1;
    int gameplay_id_ = -1;
};

} // namespace game::player