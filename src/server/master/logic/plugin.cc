

#include "plugin.h"
#include "master_module.h"

//
//

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, MasterServerPlugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, MasterServerPlugin)};

const int MasterServerPlugin::GetPluginVersion() { return 0; }

const std::string MasterServerPlugin::GetPluginName() { return GET_CLASS_NAME(MasterServerPlugin); }

void MasterServerPlugin::Install() { REGISTER_MODULE(pPluginManager, IMasterModule, MasterModule) }

void MasterServerPlugin::Uninstall() { UNREGISTER_MODULE(pPluginManager, IMasterModule, MasterModule) }
