

#include "plugin.h"
#include "http_server_module.h"
//
//

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){

    CREATE_PLUGIN(pm, MasterNet_HttpServerPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, MasterNet_HttpServerPlugin)};

const int MasterNet_HttpServerPlugin::GetPluginVersion() { return 0; }

const std::string MasterNet_HttpServerPlugin::GetPluginName() { return GET_CLASS_NAME(MasterNet_HttpServerPlugin); }

void MasterNet_HttpServerPlugin::Install() { REGISTER_MODULE(pm_, IMasterNet_HttpServerModule, MasterNet_HttpServerModule) }

void MasterNet_HttpServerPlugin::Uninstall() { UNREGISTER_MODULE(pm_, IMasterNet_HttpServerModule, MasterNet_HttpServerModule) }