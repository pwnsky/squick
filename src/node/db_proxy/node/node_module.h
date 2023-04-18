#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/kernel/i_thread_pool_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include "../logic/i_account_redis_module.h"
#include "../logic/i_player_redis_module.h"
#include "i_server_module.h"
#include <squick/plugin/net/export.h>
// #include <server/game/logic/i_scene_process_module.h>
// #include <server/world/server/i_server_module.h>
// #include <server/world/client/i_master_module.h>
namespace db_proxy::server {
class ServerModule : public IServerModule {
  public:
    ServerModule(IPluginManager *p) { pm_ = p; }

    virtual bool Awake();
    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

  protected:
    void OnClientDisconnect(const socket_t sock);
    void OnClientConnected(const socket_t sock);
    void OnLoadRoleDataProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnSaveRoleDataProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  protected:
    IPlayerRedisModule *m_pPlayerRedisModule;
    IAccountRedisModule *m_pAccountRedisModule;
};

} // namespace db_proxy::server