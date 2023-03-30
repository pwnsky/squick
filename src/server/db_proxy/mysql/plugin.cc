

#include "plugin.h"
#include "mysql_client_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, MysqlPlugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, MysqlPlugin)};

//////////////////////////////////////////////////////////////////////////

const int MysqlPlugin::GetPluginVersion() { return 0; }

const std::string MysqlPlugin::GetPluginName() { return GET_CLASS_NAME(MysqlPlugin); }

void MysqlPlugin::Install() { REGISTER_MODULE(pm_, IMysqlModule, MysqlModule) }

void MysqlPlugin::Uninstall() { UNREGISTER_MODULE(pm_, IMysqlModule, MysqlModule) }