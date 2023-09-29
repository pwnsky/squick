#pragma once

#include <squick/core/map.h>
#include "i_node_module.h"
#include <squick/plugin/net/export.h>
namespace db_proxy::node {
class NodeModule : public INodeModule {
  public:
      NodeModule(IPluginManager* p) { pm_ = p; is_update_ = true; }
    virtual bool Destory();
    virtual bool AfterStart();

  protected:
    void OnClientDisconnect(const socket_t sock);
    void OnClientConnected(const socket_t sock);

  protected:
};

} // namespace db_proxy::server