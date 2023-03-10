

#include "plugin.h"
#include "server_module.h"

//
//

SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{
    CREATE_PLUGIN(pm, LoginNet_ServerPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
    DESTROY_PLUGIN(pm, LoginNet_ServerPlugin)
};



//////////////////////////////////////////////////////////////////////////

const int LoginNet_ServerPlugin::GetPluginVersion()
{
    return 0;
}

const std::string LoginNet_ServerPlugin::GetPluginName()
{
	return GET_CLASS_NAME(LoginNet_ServerPlugin);
}

void LoginNet_ServerPlugin::Install()
{
    REGISTER_MODULE(pPluginManager, ILoginNet_ServerModule, LoginNet_ServerModule)
}

void LoginNet_ServerPlugin::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, ILoginNet_ServerModule, LoginNet_ServerModule)
}