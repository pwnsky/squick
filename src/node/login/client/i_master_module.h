#pragma once
#include <iostream>
#include <map>
#include <server/i_node_client_module.h>
#include <squick/core/i_module.h>
#include <squick/plugin/net/i_net_client_module.h>
namespace login::client {
class IMasterModule : public INodeClientModule {
  public:
    virtual std::map<int, rpc::Server> &GetServers() = 0;
};

} // namespace login::client
