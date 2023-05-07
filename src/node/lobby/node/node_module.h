#pragma once

#include "i_node_module.h"

namespace lobby::node {
class NodeModule : public INodeModule {
  public:
      NodeModule(IPluginManager *p) { pm_ = p; }
    virtual bool Destory();
    virtual bool AfterStart();

  protected:
  private:
};

} // namespace lobby::node