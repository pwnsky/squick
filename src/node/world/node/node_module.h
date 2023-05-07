#pragma once
#include "i_node_module.h"
namespace world::node {
class NodeModule : public INodeModule {
  public:
      NodeModule(IPluginManager *p) {
        pm_ = p;
    }

    virtual bool Destory();
    virtual bool AfterStart();

  protected:
    void SyncServer();

  private:
    // 同一区服，所有服务器
    map<int, ServerData> servers_;
};

} // namespace world::server