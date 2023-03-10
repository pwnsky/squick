#include "plugin.h"
#include "master_module.h"



SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{

    CREATE_PLUGIN(pm, LoginNet_ClientPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
    DESTROY_PLUGIN(pm, LoginNet_ClientPlugin)
};



const int LoginNet_ClientPlugin::GetPluginVersion()
{
    return 0;
}

const std::string LoginNet_ClientPlugin::GetPluginName()
{
	return GET_CLASS_NAME(LoginNet_ClientPlugin);
}

void LoginNet_ClientPlugin::Install()
{
    REGISTER_MODULE(pPluginManager, ILoginToMasterModule, LoginToMasterModule)
}

void LoginNet_ClientPlugin::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, ILoginToMasterModule, LoginToMasterModule)
}