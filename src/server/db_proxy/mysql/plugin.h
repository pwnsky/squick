#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>

//////////////////////////////////////////////////////////////////////////
class MysqlPlugin : public IPlugin
{
public:
    MysqlPlugin (IPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};