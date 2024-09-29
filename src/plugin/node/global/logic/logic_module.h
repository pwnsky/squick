#pragma once
#include "i_logic_module.h"
#include <plugin/core/net/export.h>
#include <plugin/core/node/export.h>

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