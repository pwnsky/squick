#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

//////////////////////////////////////////////////////////////////////////
class NoSqlPlugin : public IPlugin {
  public:
    NoSqlPlugin(IPluginManager *p) { pPluginManager = p; }

    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};