#pragma once
#include <squick/imodule/i_node_module.h>

namespace master::node {
class INodeModule : public INodeBaseModule {
  public:
    virtual map<int, ServerInfo>& GetAllNodes() = 0;
};

} // namespace master::node