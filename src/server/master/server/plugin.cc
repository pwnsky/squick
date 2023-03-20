
#include "plugin.h"
#include "server_module.h"

//
//

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, MasterNet_ServerPlugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, MasterNet_ServerPlugin)};

//////////////////////////////////////////////////////////////////////////

const int MasterNet_ServerPlugin::GetPluginVersion() { return 0; }

const std::string MasterNet_ServerPlugin::GetPluginName() { return GET_CLASS_NAME(MasterNet_ServerPlugin); }

void MasterNet_ServerPlugin::Install() { REGISTER_MODULE(pPluginManager, IMasterNet_ServerModule, MasterNet_ServerModule) }

void MasterNet_ServerPlugin::Uninstall() { UNREGISTER_MODULE(pPluginManager, IMasterNet_ServerModule, MasterNet_ServerModule) }