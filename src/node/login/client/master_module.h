#pragma once

#include <squick/plugin/kernel/export.h>

#include <squick/plugin/config/export.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/export.h>

#include "../logic/i_logic_module.h"
#include "../server/i_server_module.h"
#include "i_master_module.h"
#include <map>
namespace login::client {
class MasterModule : public IMasterModule {
  public:
    MasterModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }
    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();
    virtual bool BeforeDestory();

    virtual map<int, rpc::Server> &GetServers();

  private:
    map<int, rpc::Server> servers_;
    server::IServerModule *m_server_;
};

} // namespace login::client