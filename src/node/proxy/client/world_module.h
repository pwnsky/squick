#pragma once

#include <squick/struct/struct.h>

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/security/export.h>

#include "../logic/i_logic_module.h"
#include "../server/i_server_module.h"
#include "i_game_module.h"
#include "i_world_module.h"

namespace proxy::client {
class WorldModule : public IWorldModule {
  public:
    WorldModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }
    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

    virtual void LogReceive(const char *str) {}
    virtual void LogSend(const char *str) {}

  protected:
    void LogServerInfo(const std::string &strServerInfo);
    void OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    server::IServerModule *m_server_;
    logic::ILogicModule *m_logic_;
};

} // namespace proxy::client