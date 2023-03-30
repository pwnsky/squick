#pragma once

#include "squick/core/map.h"
#include "squick/struct/struct.h"
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_redis_module.h"

namespace login::redis {
class RedisModule : public IRedisModule {
  public:
    RedisModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool AfterStart();
protected:
    virtual bool GenerateProxyConnectKey(const Guid& guid, int proxy_id, const string& key) override;

  protected:
    INetModule *m_net_;
    ILogModule *m_log_;
  private:
};

}