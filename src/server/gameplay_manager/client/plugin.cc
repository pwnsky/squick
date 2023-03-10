
#include "plugin.h"
#include "world_module.h"
#include "game_module.h"

namespace gameplay_manager::client {
SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{

	CREATE_PLUGIN(pm, Plugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
	DESTROY_PLUGIN(pm, Plugin)
};


//////////////////////////////////////////////////////////////////////////

const int Plugin::GetPluginVersion()
{
	return 0;
}

const std::string Plugin::GetPluginName()
{
	return GET_CLASS_NAME(Plugin);
}

void Plugin::Install()
{
	REGISTER_MODULE(pPluginManager, IWorldModule, WorldModule)
	REGISTER_MODULE(pPluginManager, IGameModule, GameModule)
}

void Plugin::Uninstall()
{
	UNREGISTER_MODULE(pPluginManager, IGameModule, GameModule)
	UNREGISTER_MODULE(pPluginManager, IWorldModule, WorldModule)
}

}