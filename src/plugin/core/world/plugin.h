
#include <core/i_plugin.h>
#include <core/i_plugin_manager.h>

//////////////////////////////////////////////////////////////////////////
class WorldPlugin : public IPlugin {
  public:
    WorldPlugin(IPluginManager *p) { pm_ = p; }

    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};
