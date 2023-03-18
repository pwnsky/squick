#pragma once

#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

class MasterNet_ServerPlugin : public IPlugin {
  public:
    MasterNet_ServerPlugin(IPluginManager *p) { pPluginManager = p; }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};
