#pragma once

#include "i_logic_module.h"
#include <squick/plugin/kernel/i_kernel_module.h>
namespace master::logic {
class LogicModule : public ILogicModule {

  public:
    LogicModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

  protected:
  private:
};

} // namespace master::logic