
#include "plugin.h"
#include "log_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, LogPlugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, LogPlugin)};

//////////////////////////////////////////////////////////////////////////

const int LogPlugin::GetPluginVersion() { return 0; }

const std::string LogPlugin::GetPluginName() { return GET_CLASS_NAME(LogPlugin); }

void LogPlugin::Install() { REGISTER_MODULE(pPluginManager, ILogModule, LogModule) }

void LogPlugin::Uninstall() { UNREGISTER_MODULE(pPluginManager, ILogModule, LogModule) }