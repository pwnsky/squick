

#include "plugin.h"
#include "security_module.h"

//
//

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){

    CREATE_PLUGIN(pm, SecurityPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, SecurityPlugin)};

//////////////////////////////////////////////////////////////////////////

const int SecurityPlugin::GetPluginVersion() { return 0; }

const std::string SecurityPlugin::GetPluginName() { return GET_CLASS_NAME(SecurityPlugin); }

void SecurityPlugin::Install() { REGISTER_MODULE(pPluginManager, ISecurityModule, SecurityModule) }

void SecurityPlugin::Uninstall() { UNREGISTER_MODULE(pPluginManager, ISecurityModule, SecurityModule) }