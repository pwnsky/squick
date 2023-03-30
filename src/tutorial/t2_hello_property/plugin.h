#pragma once

#include <squick/core/base.h>

class Tutorial2 : public IPlugin {
  public:
    Tutorial2(IPluginManager *p) { pm_ = p; }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};