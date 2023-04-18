
#include "plugin.h"
#include "game_module.h"
#include "world_module.h"

namespace gameplay_manager::client {
SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){

    CREATE_PLUGIN(pm, Plugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Plugin)};

const int Plugin::GetPluginVersion() { return 0; }

const std::string Plugin::GetPluginName() { return GET_CLASS_NAME(Plugin); }

void Plugin::Install() {
    REGISTER_MODULE(pm_, IWorldModule, WorldModule)
    REGISTER_MODULE(pm_, IGameModule, GameModule)
}

void Plugin::Uninstall() {
    UNREGISTER_MODULE(pm_, IGameModule, GameModule)
    UNREGISTER_MODULE(pm_, IWorldModule, WorldModule)
}

} // namespace gameplay_manager::client