#pragma once
#include <iostream>
#include <squick/core/i_module.h>
#include <squick/plugin/net/i_net_client_module.h>

namespace login::client {
class IMasterModule : public IModule {
  public:
    virtual INetClientModule *GetClusterModule() = 0;
    virtual MapEx<int, SquickStruct::ServerInfoReport> &GetWorldMap() = 0;
};

} // namespace login::client
