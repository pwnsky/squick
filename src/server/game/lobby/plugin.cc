
#include "plugin.h"
#include "player_manager_module.h"
#include "room_module.h"
namespace game::player {
SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, Plugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Plugin)};

const int Plugin::GetPluginVersion() { return 0; }

const std::string Plugin::GetPluginName() { return GET_CLASS_NAME(Plugin); }

void Plugin::Install() {
    REGISTER_MODULE(pPluginManager, IPlayerManagerModule, PlayerManagerModule)
    REGISTER_MODULE(pPluginManager, IRoomModule, RoomModule)
}

void Plugin::Uninstall() { UNREGISTER_MODULE(pPluginManager, IRoomModule, RoomModule) }

} // namespace game::player