#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

//////////////////////////////////////////////////////////////////////////
class DBNet_ServerPlugin : public IPlugin {
  public:
    DBNet_ServerPlugin(IPluginManager *p) { pm_ = p; }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};