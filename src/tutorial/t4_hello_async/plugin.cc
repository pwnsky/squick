

#include "plugin.h"
#include "hello_async_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, Tutorial4Plugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Tutorial4Plugin)};

const int Tutorial4Plugin::GetPluginVersion() { return 0; }

const std::string Tutorial4Plugin::GetPluginName() { return GET_CLASS_NAME(Tutorial4Plugin); }

void Tutorial4Plugin::Install() { REGISTER_MODULE(pPluginManager, IHelloWorld4Module, HelloWorld4Module) }

void Tutorial4Plugin::Uninstall() { UNREGISTER_MODULE(pPluginManager, IHelloWorld4Module, HelloWorld4Module) }
