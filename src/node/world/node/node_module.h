#pragma once
#include "i_node_module.h"
namespace world::node {
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }

    virtual bool Destroy();
    virtual bool AfterStart();

  protected:
    void SyncServer();

  private:
};

} // namespace world::node