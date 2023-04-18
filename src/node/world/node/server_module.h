#pragma once

#include "squick/core/map.h"
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/kernel/i_thread_pool_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include "../client/i_master_module.h"
// #include "world_logic.h"
#include "i_server_module.h"
namespace world::server {
class ServerModule : public IServerModule {
  public:
    ServerModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

    void OnServerReport(const socket_t nFd, const int msg_id, const char *buffer, const uint32_t len);
    virtual void OnClientDisconnect(const socket_t sock) override;
    virtual void OnClientConnected(const socket_t sock) override;

  protected:
    void SyncServer();

  private:
    // 同一区服，所有服务器
    map<int, ServerData> servers_;
};

} // namespace world::server