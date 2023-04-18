// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2022-03-22
// Github: https://github.com/pwnsky/squick
// Description: gameplay server module

#pragma once

#include <squick/core/map.h>

#include "i_node_module.h"
namespace game::node {
class NodeModule : public INodeModule {
  public:
      NodeModule(IPluginManager *p) { pm_ = p; }

    virtual bool Destory();
    virtual bool AfterStart();

    virtual void OnClientDisconnect(socket_t sock) override;
    virtual void OnClientConnected(socket_t sock) override;
  protected:
  private:
};
} // namespace game::node
