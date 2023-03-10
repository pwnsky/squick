

#include "plugin.h"
#include "hello_redis_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{

    CREATE_PLUGIN(pm, Tutorial7)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
    DESTROY_PLUGIN(pm, Tutorial7)
};


const int Tutorial7::GetPluginVersion()
{
    return 0;
}

const std::string Tutorial7::GetPluginName()
{
	return GET_CLASS_NAME(Tutorial7);
}

void Tutorial7::Install()
{
    REGISTER_MODULE(pPluginManager, IHelloWorld7, HelloWorld7)
    
}

void Tutorial7::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, IHelloWorld7, HelloWorld7)
}