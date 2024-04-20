#pragma once

#include "i_node_module.h"

namespace match::node {
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }
    virtual bool Destroy();
    virtual bool AfterStart();

  protected:
  private:
};

} // namespace match::node
