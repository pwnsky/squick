#pragma once

#include "squick/core/map.h"
#include "squick/struct/struct.h"
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_logic_module.h"
#include <node/login/redis/i_redis_module.h>

namespace login::logic {
class LogicModule : public ILogicModule {
  public:
    LogicModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool AfterStart();
    virtual void OnConnectProxyVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  protected:
    INetModule *m_net_;
    redis::IRedisModule *m_redis_;

  private:
};

} // namespace login::logic