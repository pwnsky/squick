

#include "hello_plugin_module.h"
#include "plugin.h"

#ifdef SQUICK_DYNAMIC_PLUGIN

SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{

    CREATE_PLUGIN(pm, T1_HelloPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
    DESTROY_PLUGIN(pm, T1_HelloPlugin)
};

#endif
//////////////////////////////////////////////////////////////////////////

const int T1_HelloPlugin::GetPluginVersion()
{
    return 0;
}

const std::string T1_HelloPlugin::GetPluginName()
{
	return GET_CLASS_NAME(T1_HelloPlugin);
}

void T1_HelloPlugin::Install()
{
    REGISTER_MODULE(pPluginManager, IHelloPlugin, HelloWorld1)
}

void T1_HelloPlugin::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, IHelloPlugin, HelloWorld1)
}