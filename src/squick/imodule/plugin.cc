

#include "plugin.h"
#include <squick/core/i_module.h>

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, IModuolePlugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, IModuolePlugin)};

const int IModuolePlugin::GetPluginVersion() { return 0; }

const std::string IModuolePlugin::GetPluginName() { return GET_CLASS_NAME(IModuolePlugin); }

void IModuolePlugin::Install() {}

void IModuolePlugin::Uninstall() {}