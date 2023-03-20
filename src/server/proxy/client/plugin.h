#pragma once
#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

namespace proxy::client {

class Plugin : public IPlugin {
  public:
    Plugin(IPluginManager *p) { pPluginManager = p; }
    virtual const int GetPluginVersion();
    virtual const std::string GetPluginName();
    virtual void Install();
    virtual void Uninstall();
};

} // namespace proxy::client