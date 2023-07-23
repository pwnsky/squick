#pragma once
#include <squick/imodule/i_node_base_module.h>

namespace login::node {
class INodeModule : public INodeBaseModule {
  public:
  virtual std::map<int, rpc::Server> &GetServers() = 0;
};

} // namespace login::server
