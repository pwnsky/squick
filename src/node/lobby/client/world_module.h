#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <squick/struct/struct.h>

#include "../server/i_server_module.h"
#include "i_world_module.h"

namespace lobby::client {
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

  protected:
  private:
};

} // namespace lobby::client