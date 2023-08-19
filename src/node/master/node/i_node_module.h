#pragma once
#include <squick/imodule/i_node_module.h>

namespace master::node {
class INodeModule : public INodeBaseModule {
  public:
    virtual std::string GetServersStatus() = 0;
};

} // namespace master::node