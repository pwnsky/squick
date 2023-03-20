#pragma once

#include <squick/core/base.h>

class IHelloProperty : public IModule {};

class HelloProperty : public IHelloProperty {
  public:
    HelloProperty(IPluginManager *p) {
        m_bIsUpdate = true;
        pPluginManager = p;
    }

    virtual bool Start();
    virtual bool AfterStart();

    virtual bool Update();

    virtual bool BeforeDestory();
    virtual bool Destory();

  protected:
    int OnPropertyCallBackEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVarList, const SquickData &newVarList,
                                const int64_t reason);
};
