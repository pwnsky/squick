#pragma once

#include <squick/core/base.h>
#include <squick/plugin/navigation/export.h>

/*
IN THIS PLUGIN:
YOU WILL KNOW HOW TO USE THE "INavigationModule" TO FIND THE PATH FOR AI OBJECT
*/

class IHelloWorld6 : public IModule {};
class HelloWorld6 : public IHelloWorld6 {
  public:
    HelloWorld6(IPluginManager *p) { pm_ = p; }
    virtual bool AfterStart();

  protected:
    INavigationModule *m_pNavigationModule;
};