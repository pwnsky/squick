#include "plugin.h"
#include "actor_module.h"
#include <core/i_module.h>

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, ActorPlugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, ActorPlugin)};

//////////////////////////////////////////////////////////////////////////

const int ActorPlugin::GetPluginVersion() { return 0; }

const std::string ActorPlugin::GetPluginName() { return GET_CLASS_NAME(ActorPlugin); }

void ActorPlugin::Install() { REGISTER_MODULE(pm_, IActorModule, ActorModule) }

void ActorPlugin::Uninstall() { UNREGISTER_MODULE(pm_, IActorModule, ActorModule) }