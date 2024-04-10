#pragma once
#include <memory>
#include <squick/plugin/config/export.h>
#include <squick/plugin/utils/export.h>
#include <squick/plugin/world/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
// #include "squick/base/no_sql.h"
#include "i_navigation_data_module.h"

// #include "server/game/plugin/server/if_server_module.h"
// #include "server/game/plugin/client/if_world_module.h"
////////////////////////////////////////////////////////////////////////////

class NavigationDataModule : public INavigationDataModule {
  public:
    NavigationDataModule(IPluginManager *p) { pm_ = p; }
    virtual bool Start() override;
    virtual bool Destroy() override;
    virtual bool Update() override;

    virtual bool AfterStart() override;

    virtual const std::string &GetDefaultMapData(const int scene) override;

    virtual const std::shared_ptr<GroupNavigationData> GetMapData(const int scene) override;
    virtual const std::shared_ptr<GroupNavigationData> GetMapData(const int scene, const int group) override;
    virtual const std::shared_ptr<Voxel> GetMapData(const int scene, const int group, const int x, int z) override;

    // modify map data in run time
    virtual bool SetMapDataOccupyItem(const int scene, const int group, const int x, const int z, const std::string &item) override;
    virtual bool SetMapDataMovable(const int scene, const int group, const int x, int z, const int movable) override;
    virtual bool SetMapDataOccupy(const int scene, const int group, const int x, int z, const Guid occupy) override;
    virtual bool SetMapDataLayer(const int scene, const int group, const int x, int z, const int layer) override;

  protected:
    //<scene,group> ==>data
    MapEx<Guid, GroupNavigationData> mGroupNavigationData;

    //////////////////////////////////////////////////////////////////////////

    IKernelModule *m_kernel_;
    IClassModule *m_class_;
    ILogModule *m_log_;
    IElementModule *m_element_;
    INetModule *m_net_;
    IEventModule *m_event_;
    ISceneModule *m_scene_;
};
