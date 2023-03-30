#pragma once
#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

class MasterNet_HttpServerPlugin : public IPlugin {
  public:
    MasterNet_HttpServerPlugin(IPluginManager *p) { pm_ = p; }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};
