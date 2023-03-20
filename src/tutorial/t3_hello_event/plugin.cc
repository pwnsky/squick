
#include "plugin.h"
#include "hello_event_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, Tutorial3Plugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Tutorial3Plugin)};

//////////////////////////////////////////////////////////////////////////

const int Tutorial3Plugin::GetPluginVersion() { return 0; }

const std::string Tutorial3Plugin::GetPluginName() { return GET_CLASS_NAME(Tutorial3Plugin); }

void Tutorial3Plugin::Install() { REGISTER_MODULE(pPluginManager, IHelloEventModule, HelloEventModule) }

void Tutorial3Plugin::Uninstall() { UNREGISTER_MODULE(pPluginManager, IHelloEventModule, HelloEventModule) }
