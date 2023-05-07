#pragma once

#include <squick/core/map.h>
#include "../logic/i_account_redis_module.h"
#include "../logic/i_player_redis_module.h"
#include "i_node_module.h"
#include <squick/plugin/net/export.h>
namespace db_proxy::node {
class NodeModule : public INodeModule {
  public:
      NodeModule(IPluginManager *p) { pm_ = p; }
    virtual bool Destory();
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