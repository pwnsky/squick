

#include "plugin.h"


SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{
    CREATE_PLUGIN(pm, MysqlPlugin)
};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
    DESTROY_PLUGIN(pm, MysqlPlugin)
};


//////////////////////////////////////////////////////////////////////////

const int MysqlPlugin::GetPluginVersion()
{
    return 0;
}

const std::string MysqlPlugin::GetPluginName()
{
	return GET_CLASS_NAME(MysqlPlugin);
}

void MysqlPlugin::Install()
{
	//REGISTER_MODULE(pPluginManager, IMysqlPlugin, NoSqlModule)
}

void MysqlPlugin::Uninstall()
{
	//UNREGISTER_MODULE(pPluginManager, INoSqlModule, NoSqlModule)
}