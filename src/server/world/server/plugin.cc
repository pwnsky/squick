
#include "plugin.h"
#include "server_module.h"

//
//
#ifdef SQUICK_DYNAMIC_PLUGIN

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, WorldNet_ServerPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, WorldNet_ServerPlugin)};

#endif
//////////////////////////////////////////////////////////////////////////

const int WorldNet_ServerPlugin::GetPluginVersion() { return 0; }

const std::string WorldNet_ServerPlugin::GetPluginName() { return GET_CLASS_NAME(WorldNet_ServerPlugin); }

void WorldNet_ServerPlugin::Install() { REGISTER_MODULE(pPluginManager, IWorldNet_ServerModule, WorldNet_ServerModule) }

void WorldNet_ServerPlugin::Uninstall() { UNREGISTER_MODULE(pPluginManager, IWorldNet_ServerModule, WorldNet_ServerModule) }