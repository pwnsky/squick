

#include "plugin.h"
#include "server_module.h"

//
//
#ifdef SQUICK_DYNAMIC_PLUGIN

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, DBNet_ServerPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, DBNet_ServerPlugin)};

#endif
//////////////////////////////////////////////////////////////////////////

const int DBNet_ServerPlugin::GetPluginVersion() { return 0; }

const std::string DBNet_ServerPlugin::GetPluginName() { return GET_CLASS_NAME(DBNet_ServerPlugin); }

void DBNet_ServerPlugin::Install() { REGISTER_MODULE(pPluginManager, IDBNet_ServerModule, DBNet_ServerModule) }

void DBNet_ServerPlugin::Uninstall() { UNREGISTER_MODULE(pPluginManager, IDBNet_ServerModule, DBNet_ServerModule) }