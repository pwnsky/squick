// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2022-04-04
// Github: https://github.com/pwnsky/squick
// Description: robot server module

#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_node_module.h"
namespace robot::server {
class ServerModule : public IServerModule {
  public:
    ServerModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

  protected:
  private:
};
} // namespace robot::server
