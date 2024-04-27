#pragma once
#include "i_node_module.h"
#include <set>

namespace master::node {
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager *p) { pm_ = p; }
    virtual ~NodeModule();
    virtual bool Destroy();
    virtual bool AfterStart();
};

} // namespace master::node