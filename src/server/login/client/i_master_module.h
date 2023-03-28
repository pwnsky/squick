#pragma once
#include <iostream>
#include <squick/core/i_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <map>
namespace login::client {
class IMasterModule : public IModule {
  public:
    virtual INetClientModule *GetClusterModule() = 0;
    virtual std::map<int, SquickStruct::ServerInfoReport> & GetProxyServers() = 0;
    virtual std::map<int, SquickStruct::ServerInfoReport>& GetWorldServers() = 0;
};

} // namespace login::client
