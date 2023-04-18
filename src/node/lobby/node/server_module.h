#pragma once

#include <memory>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include <squick/plugin/kernel/i_event_module.h>
#include <squick/plugin/kernel/i_scene_module.h>
#include <squick/plugin/kernel/i_schedule_module.h>
#include <squick/plugin/kernel/i_thread_pool_module.h>

#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>

#include "../client/i_world_module.h"
#include "i_server_module.h"
#include <squick/core/i_module.h>
////////////////////////////////////////////////////////////////////////////

namespace lobby::server {
class ServerModule : public IServerModule {
  public:
    ServerModule(IPluginManager *p) { pm_ = p; }
    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

  protected:
    void OnClientDisconnect(const socket_t sock);
    void OnClientConnected(const socket_t sock);

  protected:
  private:
};

} // namespace lobby::server