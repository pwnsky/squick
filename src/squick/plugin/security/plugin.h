#pragma once

///
#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

class SecurityPlugin : public IPlugin {
  public:
    SecurityPlugin(IPluginManager *p) { pm_ = p; }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};