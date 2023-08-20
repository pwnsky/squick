#pragma once
#include <squick/imodule/i_node_module.h>

namespace login::node {
class INodeModule : public INodeBaseModule {
  public:
  virtual std::map<int, ServerInfo> &GetServers() = 0;
};

} // namespace login::server
