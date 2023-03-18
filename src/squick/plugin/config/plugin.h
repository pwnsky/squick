
#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>
//////////////////////////////////////////////////////////////////////////
class ConfigPlugin : public IPlugin {
  public:
    ConfigPlugin(IPluginManager *p);

    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};
