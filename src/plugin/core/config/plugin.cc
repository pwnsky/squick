
#include "plugin.h"
#include "class_module.h"
#include "config_module.h"
#include "element_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, ConfigPlugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, ConfigPlugin)};

//////////////////////////////////////////////////////////////////////////
ConfigPlugin::ConfigPlugin(IPluginManager *p) { pm_ = p; }

const int ConfigPlugin::GetPluginVersion() { return 0; }

const std::string ConfigPlugin::GetPluginName() { return GET_CLASS_NAME(ConfigPlugin); }

void ConfigPlugin::Install() {
    REGISTER_MODULE(pm_, IClassModule, ClassModule)
    REGISTER_MODULE(pm_, IElementModule, ElementModule)
    REGISTER_MODULE(pm_, ICommonConfigModule, ConfigModule);
}

void ConfigPlugin::Uninstall() {
    UNREGISTER_MODULE(pm_, IElementModule, ElementModule)
    UNREGISTER_MODULE(pm_, IClassModule, ClassModule)
    UNREGISTER_MODULE(pm_, ICommonConfigModule, ConfigModule);
}
