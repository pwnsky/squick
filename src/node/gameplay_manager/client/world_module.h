#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include <squick/core/i_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/struct/struct.h>

namespace gameplay_manager::client {

class IWorldModule : public IModule {
  public:
};

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
    virtual bool BeforeDestory();

  protected:
  protected:
  private:
};

} // namespace gameplay_manager::client