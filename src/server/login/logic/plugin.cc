

#include "plugin.h"
#include "logic_module.h"

//

SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{
    CREATE_PLUGIN(pm, LoginLogicPlugin)
};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
    DESTROY_PLUGIN(pm, LoginLogicPlugin)
};

//////////////////////////////////////////////////////////////////////////

const int LoginLogicPlugin::GetPluginVersion()
{
    return 0;
}

const std::string LoginLogicPlugin::GetPluginName()
{
    return GET_CLASS_NAME(LoginLogicPlugin);
}

void LoginLogicPlugin::Install()
{

    REGISTER_MODULE(pPluginManager, ILoginLogicModule, LoginLogicModule)

}

void LoginLogicPlugin::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, ILoginLogicModule, LoginLogicModule)
}
