
#include "plugin.h"
#include "hello_property_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){

    CREATE_PLUGIN(pm, Tutorial2)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Tutorial2)};

//////////////////////////////////////////////////////////////////////////

const int Tutorial2::GetPluginVersion() { return 0; }

const std::string Tutorial2::GetPluginName() { return GET_CLASS_NAME(Tutorial2); }

void Tutorial2::Install() { REGISTER_MODULE(pm_, IHelloProperty, HelloProperty) }

void Tutorial2::Uninstall() { UNREGISTER_MODULE(pm_, IHelloProperty, HelloProperty) }