


#include "plugin.h"
#include "world_module.h"
#include "db_module.h"


SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{
    CREATE_PLUGIN(pm, GameServerNet_ClientPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
    DESTROY_PLUGIN(pm, GameServerNet_ClientPlugin)
};


//////////////////////////////////////////////////////////////////////////

const int GameServerNet_ClientPlugin::GetPluginVersion()
{
    return 0;
}

const std::string GameServerNet_ClientPlugin::GetPluginName()
{
	return GET_CLASS_NAME(GameServerNet_ClientPlugin);
}

void GameServerNet_ClientPlugin::Install()
{
	REGISTER_MODULE(pPluginManager, IGameServerToWorldModule, GameServerToWorldModule)
	REGISTER_MODULE(pPluginManager, IGameServerToDBModule, GameServerToDBModule)

}

void GameServerNet_ClientPlugin::Uninstall()
{
	UNREGISTER_MODULE(pPluginManager, IGameServerToDBModule, GameServerToDBModule)
	UNREGISTER_MODULE(pPluginManager, IGameServerToWorldModule, GameServerToWorldModule)
}