

#include "plugin.h"
#include "db_module.h"
#include "world_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, GameServerNet_ClientPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, GameServerNet_ClientPlugin)};

//////////////////////////////////////////////////////////////////////////

const int GameServerNet_ClientPlugin::GetPluginVersion() { return 0; }

const std::string GameServerNet_ClientPlugin::GetPluginName() { return GET_CLASS_NAME(GameServerNet_ClientPlugin); }

void GameServerNet_ClientPlugin::Install() {
    REGISTER_MODULE(pm_, IGameServerToWorldModule, GameServerToWorldModule)
    REGISTER_MODULE(pm_, IGameServerToDBModule, GameServerToDBModule)
}

void GameServerNet_ClientPlugin::Uninstall() {
    UNREGISTER_MODULE(pm_, IGameServerToDBModule, GameServerToDBModule)
    UNREGISTER_MODULE(pm_, IGameServerToWorldModule, GameServerToWorldModule)
}