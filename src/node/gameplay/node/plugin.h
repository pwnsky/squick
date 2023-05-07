#pragma once
#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

namespace gameplay::node {
class Plugin : public IPlugin {
  public:
    Plugin(IPluginManager *p) { pm_ = p; }
    virtual const int GetPluginVersion();
    virtual const std::string GetPluginName();
    virtual void Install();
    virtual void Uninstall();
};

} // namespace gameplay::server
