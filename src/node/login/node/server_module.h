#pragma once
#include "squick/core/map.h"

#include <squick/core/base.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "../client/i_master_module.h"
#include "../logic/i_logic_module.h"
#include "i_server_module.h"

#include "squick/struct/struct.h"

namespace login::server {
class ServerModule : public IServerModule {

  public:
    ServerModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool BeforeDestory();
    virtual bool AfterStart();

  protected:
    virtual void OnClientDisconnect(const socket_t sock) override;
    virtual void OnClientConnected(const socket_t sock) override;
    void InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
};

} // namespace login::server