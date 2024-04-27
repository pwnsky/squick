#pragma once
#include "i_node_module.h"
#include <set>

namespace master::node {
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager* p) { pm_ = p; is_update_ = true; }
    virtual ~NodeModule();
    virtual bool Destroy();
    virtual bool AfterStart();
    virtual ServerInfo* GetNodeInfo() override;
};

} // namespace master::node