#pragma once

#include <core/base.h>
namespace tutorial {

class ISimpleModule : public IModule {};

class SimpleModule : public ISimpleModule {
  public:
    SimpleModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }
    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();

  private:
};

} // namespace tutorial