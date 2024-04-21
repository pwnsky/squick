#pragma once

#include <squick/core/base.h>

#include "game.h"
#include "i_game_mgr_module.h"
#include <queue>
#include <struct/struct.h>

namespace game::logic {

// Factory game manager
class GameMgrModule : public IGameMgrModule {
  public:
    GameMgrModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true; // Update
    }

    virtual ~GameMgrModule(){};
    virtual bool Start();
    virtual bool Destroy();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool AfterStart();
    virtual bool GameCreate(int id, const string &key) override;
    virtual bool GameDestroy(int id) override;
    virtual bool DoGamePlayerQuit(const Guid &player) override;
    virtual bool SingleGameCreate(int id, const string &key) override;
    virtual bool SingleGameDestroy(int id) override;

    virtual void OnRecv(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) override;

  private:
    std::unordered_map<int, IGame *> m_gameplay;
    std::vector<int> gameplayWaitDestroy;
};

} // namespace game::logic