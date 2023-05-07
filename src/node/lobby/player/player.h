
// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-01-05
// Description: 玩家进入游戏后，由玩家管理器创建出来的对象
#include "i_player.h"

#pragma once
namespace lobby::player {

class Player : public IPlayer {
  public:
    Player();
    virtual ~Player();
    void OnEnterGame();
    void OnOffline();
    void OnDestroy();
    void OnEnterScene();
    void OnExitScene();
    void OnChangeScene();

    int glove_ = 0;
    int mask_ = 0;
};

} // namespace lobby::player