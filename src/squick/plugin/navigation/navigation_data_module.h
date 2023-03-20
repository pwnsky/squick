#pragma once
#include <memory>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_event_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/kernel/i_scene_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>
// #include "squick/base/no_sql.h"
#include "i_navigation_data_module.h"

// #include "server/game/plugin/server/if_server_module.h"
// #include "server/game/plugin/client/if_world_module.h"
////////////////////////////////////////////////////////////////////////////

class NavigationDataModule : public INavigationDataModule {
  public:
    NavigationDataModule(IPluginManager *p) { pPluginManager = p; }
    virtual bool Start() override;
    virtual bool Destory() override;
    virtual bool Update() override;

    virtual bool AfterStart() override;

    virtual const std::string &GetDefaultMapData(const int scene) override;

    virtual const SQUICK_SHARE_PTR<GroupNavigationData> GetMapData(const int scene) override;
    virtual const SQUICK_SHARE_PTR<GroupNavigationData> GetMapData(const int scene, const int group) override;
    virtual const SQUICK_SHARE_PTR<Voxel> GetMapData(const int scene, const int group, const int x, int z) override;

    // modify map data in run time
    virtual bool SetMapDataOccupyItem(const int scene, const int group, const int x, const int z, const std::string &item) override;
    virtual bool SetMapDataMovable(const int scene, const int group, const int x, int z, const int movable) override;
    virtual bool SetMapDataOccupy(const int scene, const int group, const int x, int z, const Guid occupy) override;
    virtual bool SetMapDataLayer(const int scene, const int group, const int x, int z, const int layer) override;

  protected:
    //<scene,group> ==>data
    MapEx<Guid, GroupNavigationData> mGroupNavigationData;

    //////////////////////////////////////////////////////////////////////////

    IKernelModule *m_pKernelModule;
    IClassModule *m_pClassModule;
    ILogModule *m_pLogModule;
    IElementModule *m_pElementModule;
    INetModule *m_pNetModule;
    IEventModule *m_pEventModule;
    ISceneModule *m_pSceneModule;
};
