

#include "plugin.h"
#include "node_module.h"

namespace gameplay::node {
SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, Plugin)};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Plugin)};

const int Plugin::GetPluginVersion() { return 0; }

const std::string Plugin::GetPluginName() { return GET_CLASS_NAME(Plugin); }

void Plugin::Install() { REGISTER_MODULE(pm_, INodeModule, NodeModule) }

void Plugin::Uninstall() { UNREGISTER_MODULE(pm_, INodeModule, NodeModule) }

} // namespace gameplay::node