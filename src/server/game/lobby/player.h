
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

    int GetRoomID() { return roomId; }

    void SetRoomID(int id) { this->roomId = id; }

    void SetGameplayID(int id) { this->gameplayId = id; }

    int GetGameplayID() { return this->gameplayId; }

    int offlineTime = 0;
    int loginTime = 0;

  private:
    int roomId = -1;
    int gameplayId = -1;
};

} // namespace game::player