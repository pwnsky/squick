#pragma once

#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

class DBLogicPlugin : public IPlugin {
  public:
    DBLogicPlugin(IPluginManager *p) { pm_ = p; }

    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};