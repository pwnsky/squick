
#include "plugin.h"
#include "cell_module.h"
#include "data_tail_module.h"
#include "elo_module.h"
#include "event_module.h"
#include "kernel_module.h"
#include "scene_module.h"
#include "schedule_module.h"
#include "thread_pool_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, WorldPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, WorldPlugin)};

//////////////////////////////////////////////////////////////////////////

const int WorldPlugin::GetPluginVersion() { return 0; }

const std::string WorldPlugin::GetPluginName() { return GET_CLASS_NAME(WorldPlugin); }

void WorldPlugin::Install() {
    REGISTER_MODULE(pm_, ISceneModule, SceneModule)
    REGISTER_MODULE(pm_, IKernelModule, KernelModule)
    REGISTER_MODULE(pm_, IEventModule, EventModule)
    REGISTER_MODULE(pm_, IScheduleModule, ScheduleModule)
    REGISTER_MODULE(pm_, IDataTailModule, DataTailModule)
    REGISTER_MODULE(pm_, ICellModule, CellModule)
    REGISTER_MODULE(pm_, IThreadPoolModule, ThreadPoolModule)
    REGISTER_MODULE(pm_, IELOModule, ELOModule)
}

void WorldPlugin::Uninstall() {

    UNREGISTER_MODULE(pm_, IELOModule, ELOModule)
    UNREGISTER_MODULE(pm_, IThreadPoolModule, ThreadPoolModule)
    UNREGISTER_MODULE(pm_, ICellModule, CellModule)
    UNREGISTER_MODULE(pm_, IDataTailModule, DataTailModule)
    UNREGISTER_MODULE(pm_, IEventModule, EventModule)
    UNREGISTER_MODULE(pm_, IKernelModule, KernelModule)
    UNREGISTER_MODULE(pm_, ISceneModule, SceneModule)
    UNREGISTER_MODULE(pm_, IScheduleModule, ScheduleModule)
}