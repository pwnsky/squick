#pragma once
#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>



class LoginNet_ClientPlugin : public IPlugin
{
public:
    LoginNet_ClientPlugin(IPluginManager* p)
    {
        pPluginManager = p;
    }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};
