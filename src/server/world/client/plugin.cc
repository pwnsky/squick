
#include "plugin.h"
#include "master_module.h"

//
//

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){

    CREATE_PLUGIN(pm, WorldNet_ClientPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, WorldNet_ClientPlugin)};

//////////////////////////////////////////////////////////////////////////

const int WorldNet_ClientPlugin::GetPluginVersion() { return 0; }

const std::string WorldNet_ClientPlugin::GetPluginName() { return GET_CLASS_NAME(WorldNet_ClientPlugin); }

void WorldNet_ClientPlugin::Install() { REGISTER_MODULE(pm_, IWorldToMasterModule, WorldToMasterModule) }

void WorldNet_ClientPlugin::Uninstall() { UNREGISTER_MODULE(pm_, IWorldToMasterModule, WorldToMasterModule) }