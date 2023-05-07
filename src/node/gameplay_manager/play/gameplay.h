#pragma once

#include "i_gameplay.h"
#include <list>
#include <vector>

namespace gameplay_manager::play {

class Gameplay : public IGameplay {
  public:
    Gameplay();
    virtual ~Gameplay();
    virtual void Awake() override;
    virtual void Start() override;
    virtual void Update() override;
    virtual void Destroy() override;

    virtual void PlayerJoin(const Guid &player) override;
    virtual void PlayerQuit(const Guid &player) override;
    virtual void AllPlayerJoined() override;
};

} // namespace game::play