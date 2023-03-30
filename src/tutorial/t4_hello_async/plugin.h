#pragma once

#include <squick/core/base.h>

class Tutorial4Plugin : public IPlugin {
  public:
    Tutorial4Plugin(IPluginManager *p) { pm_ = p; }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};