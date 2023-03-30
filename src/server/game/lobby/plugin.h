#pragma once

#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

namespace game::player {
class Plugin : public IPlugin {
  public:
    Plugin(IPluginManager *p) { pm_ = p; }
    virtual const int GetPluginVersion();
    virtual const std::string GetPluginName();
    virtual void Install();
    virtual void Uninstall();
};

} // namespace game::player