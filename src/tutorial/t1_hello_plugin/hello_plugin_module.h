#pragma once

#include <squick/core/base.h>
class IHelloPlugin : public IModule {};

class HelloWorld1 : public IHelloPlugin {
  public:
    HelloWorld1(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }

    virtual bool Start();
    virtual bool AfterStart();

    virtual bool Update();

    virtual bool BeforeDestory();
    virtual bool Destory();

  protected:
};
