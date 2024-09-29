#pragma once

#include "i_game.h"
#include <list>
#include <vector>

namespace game::logic {

class Game : public IGame {
  public:
    Game();
    virtual ~Game();
    virtual void Awake() override;
    virtual void Start() override;
    virtual void Update() override;
    virtual void Destroy() override;

    virtual void PlayerJoin(const Guid &player) override;
    virtual void PlayerQuit(const Guid &player) override;
    virtual void AllPlayerJoined() override;
};

} // namespace game::logic