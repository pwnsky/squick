
#pragma once
#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>


//////////////////////////////////////////////////////////////////////////
class LogPlugin : public IPlugin
{
public:
    LogPlugin(IPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};
