#include "plugin.h"

#include "event_module.h"
#include "schedule_module.h"
#include "thread_pool_module.h"

namespace tutorial {
SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, Plugin)};
SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, Plugin)};

const int Plugin::GetPluginVersion() { return 0; }
const std::string Plugin::GetPluginName() { return GET_CLASS_NAME(Plugin); }
void Plugin::Install() {
    REGISTER_MODULE(pm_, IEventModule, EventModule)
    REGISTER_MODULE(pm_, IScheduleModule, ScheduleModule)
    REGISTER_MODULE(pm_, IThreadPoolModule, ThreadPoolModule)
}
void Plugin::Uninstall() {
    UNREGISTER_MODULE(pm_, IThreadPoolModule, IThreadPoolModule)
    UNREGISTER_MODULE(pm_, IScheduleModule, ScheduleModule)
    UNREGISTER_MODULE(pm_, IEventModule, EventModule)
}
} // namespace tutorial