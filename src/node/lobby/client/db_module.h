#pragma once
#include "squick/struct/struct.h"
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/net/i_net_module.h>

#include "../server/i_server_module.h"
#include "i_db_module.h"

namespace lobby::client {
class DBModule : public IDBModule {
  public:
    DBModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();
};

} // namespace lobby::client