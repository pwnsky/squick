

#include "plugin.h"
#include "redis_client_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, NoSqlPlugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, NoSqlPlugin)};

//////////////////////////////////////////////////////////////////////////

const int NoSqlPlugin::GetPluginVersion() { return 0; }

const std::string NoSqlPlugin::GetPluginName() { return GET_CLASS_NAME(NoSqlPlugin); }

void NoSqlPlugin::Install() { REGISTER_MODULE(pm_, IRedisModule, RedisModule) }

void NoSqlPlugin::Uninstall() { UNREGISTER_MODULE(pm_, IRedisModule, RedisModule) }