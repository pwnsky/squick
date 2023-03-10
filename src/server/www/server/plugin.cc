

#include "plugin.h"
#include "http_server_module.h"
//
//

SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{

	CREATE_PLUGIN(pm, squick::server::www::plugin::server::Plugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
	DESTROY_PLUGIN(pm, squick::server::www::plugin::server::Plugin)
};

// squick::server::www::plugin::server::module
namespace squick::server::www::plugin::server {


const int Plugin::GetPluginVersion()
{
	return 0;
}

const std::string Plugin::GetPluginName()
{
	return GET_CLASS_NAME(Plugin);
}

void Plugin::Install()
{
	REGISTER_MODULE(pPluginManager, module::HttpServer, module::HttpServer)
}

void Plugin::Uninstall()
{
	UNREGISTER_MODULE(pPluginManager, module::HttpServer, module::HttpServer)
}

}