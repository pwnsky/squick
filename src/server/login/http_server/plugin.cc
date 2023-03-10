

#include "plugin.h"
#include "http_server_module.h"



SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{

	CREATE_PLUGIN(pm, LoginNet_HttpServerPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
	DESTROY_PLUGIN(pm, LoginNet_HttpServerPlugin)
};


//////////////////////////////////////////////////////////////////////////

const int LoginNet_HttpServerPlugin::GetPluginVersion()
{
	return 0;
}

const std::string LoginNet_HttpServerPlugin::GetPluginName()
{
	return GET_CLASS_NAME(LoginNet_HttpServerPlugin);
}

void LoginNet_HttpServerPlugin::Install()
{
	REGISTER_MODULE(pPluginManager, ILoginNet_HttpServerModule, LoginNet_HttpServerModule)
}

void LoginNet_HttpServerPlugin::Uninstall()
{
	UNREGISTER_MODULE(pPluginManager, ILoginNet_HttpServerModule, LoginNet_HttpServerModule)
}