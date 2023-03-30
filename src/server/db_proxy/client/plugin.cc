

#include "plugin.h"
#include "world_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){

    CREATE_PLUGIN(pm, Net_ClientPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Net_ClientPlugin)};

//////////////////////////////////////////////////////////////////////////

const int Net_ClientPlugin::GetPluginVersion() { return 0; }

const std::string Net_ClientPlugin::GetPluginName() { return GET_CLASS_NAME(Net_ClientPlugin); }

void Net_ClientPlugin::Install() { REGISTER_MODULE(pm_, IDBToWorldModule, DBToWorldModule) }

void Net_ClientPlugin::Uninstall() { UNREGISTER_MODULE(pm_, IDBToWorldModule, DBToWorldModule) }