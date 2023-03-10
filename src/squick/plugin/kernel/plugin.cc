
#include "plugin.h"
#include "kernel_module.h"
#include "scene_module.h"
#include "event_module.h"
#include "schedule_module.h"
#include "data_tail_module.h"
#include "cell_module.h"
#include "thread_pool_module.h"
#include "elo_module.h"



SQUICK_EXPORT void SquickPluginLoad(IPluginManager* pm)
{
    CREATE_PLUGIN(pm, KernelPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager* pm)
{
    DESTROY_PLUGIN(pm, KernelPlugin)
};


//////////////////////////////////////////////////////////////////////////

const int KernelPlugin::GetPluginVersion()
{
    return 0;
}

const std::string KernelPlugin::GetPluginName()
{
	return GET_CLASS_NAME(KernelPlugin);
}

void KernelPlugin::Install()
{
    REGISTER_MODULE(pPluginManager, ISceneModule, SceneModule)
	REGISTER_MODULE(pPluginManager, IKernelModule, KernelModule)
	REGISTER_MODULE(pPluginManager, IEventModule, EventModule)
	REGISTER_MODULE(pPluginManager, IScheduleModule, ScheduleModule)
	REGISTER_MODULE(pPluginManager, IDataTailModule, DataTailModule)
	REGISTER_MODULE(pPluginManager, ICellModule, CellModule)
	REGISTER_MODULE(pPluginManager, IThreadPoolModule, ThreadPoolModule)
	REGISTER_MODULE(pPluginManager, IELOModule, ELOModule)

}

void KernelPlugin::Uninstall()
{
	
	UNREGISTER_MODULE(pPluginManager, IELOModule, ELOModule)
	UNREGISTER_MODULE(pPluginManager, IThreadPoolModule, ThreadPoolModule)
	UNREGISTER_MODULE(pPluginManager, ICellModule, CellModule)
	UNREGISTER_MODULE(pPluginManager, IDataTailModule, DataTailModule)
	UNREGISTER_MODULE(pPluginManager, IEventModule, EventModule)
	UNREGISTER_MODULE(pPluginManager, IKernelModule, KernelModule)
	UNREGISTER_MODULE(pPluginManager, ISceneModule, SceneModule)
	UNREGISTER_MODULE(pPluginManager, IScheduleModule, ScheduleModule)

}