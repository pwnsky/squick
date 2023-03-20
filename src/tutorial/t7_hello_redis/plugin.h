#pragma once

#include <squick/core/base.h>

class Tutorial7 : public IPlugin {
  public:
    Tutorial7(IPluginManager *p) { pPluginManager = p; }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};