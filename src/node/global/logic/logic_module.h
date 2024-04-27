#pragma once
#include "i_logic_module.h"
#include <squick/plugin/net/export.h>
#include <squick/plugin/node/export.h>

namespace global::logic {
class LogicModule : public ILogicModule {
  public:
    LogicModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true; // Update
    }
    virtual bool Start() override;
  private:
    INodeModule *m_node_;
};

} // namespace global::logic