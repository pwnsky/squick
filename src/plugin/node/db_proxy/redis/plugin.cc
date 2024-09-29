#include "plugin.h"
#include "redis_module.h"
namespace db_proxy::redis {

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, Plugin)};
SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Plugin)};

const int Plugin::GetPluginVersion() { return 0; }

const std::string Plugin::GetPluginName() { return GET_CLASS_NAME(Plugin); }

void Plugin::Install() { REGISTER_MODULE(pm_, IRedisModule, RedisModule) }

void Plugin::Uninstall() { UNREGISTER_MODULE(pm_, IRedisModule, RedisModule) }

} // namespace db_proxy::redis