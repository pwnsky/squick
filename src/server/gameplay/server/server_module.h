// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2022-03-22
// Github: https://github.com/pwnsky/squick
// Description: gameplay server module

#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_server_module.h"
namespace gameplay::server {
class ServerModule : public IServerModule {
  public:
    ServerModule(IPluginManager *p) { pPluginManager = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

  protected:
  private:
};
} // namespace gameplay::server
