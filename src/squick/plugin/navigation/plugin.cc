

#include "navigation_module.h"
#include "navigation_data_module.h"
#include "plugin.h"

//
//

SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{
    CREATE_PLUGIN(pm, NavigationPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
    DESTROY_PLUGIN(pm, NavigationPlugin)
};



//////////////////////////////////////////////////////////////////////////

const int NavigationPlugin::GetPluginVersion()
{
    return 0;
}

const std::string NavigationPlugin::GetPluginName()
{
	return GET_CLASS_NAME(NavigationPlugin);
}

void NavigationPlugin::Install()
{
    REGISTER_MODULE(pPluginManager, INavigationModule, NavigationModule)
    REGISTER_MODULE(pPluginManager, INavigationDataModule, NavigationDataModule)
}

void NavigationPlugin::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, INavigationDataModule, NavigationDataModule)
    UNREGISTER_MODULE(pPluginManager, INavigationModule, NavigationModule)
}