

#include "plugin.h"
#include "hello_navigation.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){

    CREATE_PLUGIN(pm, Tutorial6)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Tutorial6)};

//////////////////////////////////////////////////////////////////////////

const int Tutorial6::GetPluginVersion() { return 0; }

const std::string Tutorial6::GetPluginName() { return GET_CLASS_NAME(Tutorial6); }

void Tutorial6::Install() { REGISTER_MODULE(pm_, IHelloWorld6, HelloWorld6) }

void Tutorial6::Uninstall() { UNREGISTER_MODULE(pm_, IHelloWorld6, HelloWorld6) }