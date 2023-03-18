

#include "plugin.h"
#include "hello_http_server_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){

    CREATE_PLUGIN(pm, Tutorial5)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Tutorial5)};

//////////////////////////////////////////////////////////////////////////

const int Tutorial5::GetPluginVersion() { return 0; }

const std::string Tutorial5::GetPluginName() { return GET_CLASS_NAME(Tutorial5); }

void Tutorial5::Install() { REGISTER_MODULE(pPluginManager, IHelloWorld5, HelloWorld5) }

void Tutorial5::Uninstall() { UNREGISTER_MODULE(pPluginManager, IHelloWorld5, HelloWorld5) }