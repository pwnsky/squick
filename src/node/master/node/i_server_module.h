#pragma once

#include <iostream>
#include <server/i_node_server_module.h>
#include <squick/core/i_module.h>

namespace master::server {
class IServerModule : public INodeServerModule {
  public:
    virtual std::string GetServersStatus() = 0;
};

} // namespace master::server