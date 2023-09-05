

#include "plugin.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, DBLogicPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, DBLogicPlugin)};

//////////////////////////////////////////////////////////////////////////

const int DBLogicPlugin::GetPluginVersion() { return 0; }

const std::string DBLogicPlugin::GetPluginName() { return GET_CLASS_NAME(DBLogicPlugin); }

void DBLogicPlugin::Install() {
}

void DBLogicPlugin::Uninstall() {
}