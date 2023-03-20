
#include "plugin.h"
#include "logic_module.h"
#include "manager_module.h"
namespace gameplay_manager::logic {
SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){

    CREATE_PLUGIN(pm, Plugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Plugin)};

//////////////////////////////////////////////////////////////////////////

const int Plugin::GetPluginVersion() { return 0; }

const std::string Plugin::GetPluginName() { return GET_CLASS_NAME(Plugin); }

void Plugin::Install() {
    REGISTER_MODULE(pPluginManager, IManagerModule, ManagerModule)
    REGISTER_MODULE(pPluginManager, ILogicModule, LogicModule)
}

void Plugin::Uninstall() {
    UNREGISTER_MODULE(pPluginManager, ILogicModule, LogicModule)
    UNREGISTER_MODULE(pPluginManager, IManagerModule, ManagerModule)
}

} // namespace gameplay_manager::logic