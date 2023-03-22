#pragma once

#include <squick/core/base.h>

#include "gameplay.h"

#include "i_gameplay_manager_module.h"
#include <queue>

namespace game::play {

// 采用工厂模式来进行管理gameplay
class GameplayManagerModule : public IGameplayManagerModule {
  public:
    GameplayManagerModule(IPluginManager *p) {
        pPluginManager = p;
        m_bIsUpdate = true; // Update
    }

    virtual ~GameplayManagerModule(){};
    virtual bool Start();
    virtual bool Destory();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool AfterStart();
    virtual bool GameplayCreate(int id, const string &key) override;
    virtual bool GameplayDestroy(int id) override;
    virtual bool GameplayPlayerQuit(const Guid &player) override;
    virtual void OnRecv(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) override;

  private:
    std::map<int, IGameplay *> m_gameplay;
    std::vector<int> gameplayWaitDestroy;
};

} // namespace game::play