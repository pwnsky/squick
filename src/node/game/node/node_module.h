// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2022-03-22
// Github: https://github.com/pwnsky/squick
// Description: gameplay server module

#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_node_module.h"
namespace gameplay::node {
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager *p) { pm_ = p; }

    virtual bool Destroy();
    virtual bool AfterStart();

  protected:
  private:
};
} // namespace gameplay::node
