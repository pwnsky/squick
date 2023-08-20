#pragma once

#include <squick/core/base.h>

#include "gameplay.h"

#include "i_gameplay_manager_module.h"
#include <queue>
#include <struct/struct.h>

namespace gameplay_manager::play {

// 采用工厂模式来进行管理gameplay
class GameplayManagerModule : public IGameplayManagerModule {
  public:
    GameplayManagerModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true; // Update
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
    virtual bool SingleGameplayCreate(int id, const string &key) override;
    virtual bool SingleGameplayDestroy(int id) override;

    virtual void OnRecv(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) override;

  private:
    std::unordered_map<int, IGameplay *> m_gameplay;
    std::vector<int> gameplayWaitDestroy;
};

} // namespace game::play