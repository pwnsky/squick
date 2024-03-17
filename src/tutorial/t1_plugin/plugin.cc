#include "plugin.h"
#include "simple_module.h"
namespace tutorial {
    // Suiqck can load or unload this plugin by these functions.
    SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm) { CREATE_PLUGIN(pm, Plugin) };
    SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm) { DESTROY_PLUGIN(pm, Plugin) };

    const int Plugin::GetPluginVersion() { return 0; }
    const std::string Plugin::GetPluginName() { return GET_CLASS_NAME(Plugin); }
    void Plugin::Install() {
        
        // Here is register your own module in this plugin
        REGISTER_MODULE(pm_, ISimpleModule, SimpleModule)
        REGISTER_MODULE(pm_, IModule, CallerModule)
    }
    void Plugin::Uninstall() {
        // Here is unregister your own module when plugin unload
        // The order of calls is opposite to when loaded
        UNREGISTER_MODULE(pm_, IModule, CallerModule)
        UNREGISTER_MODULE(pm_, ISimpleModule, SimpleModule)
    }
}