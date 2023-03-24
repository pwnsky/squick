#pragma once

#include "squick/core/map.h"
#include "squick/struct/struct.h"
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_redis_module.h"

namespace proxy::redis {
class RedisModule : public IRedisModule {
  public:
    RedisModule(IPluginManager *p) { pPluginManager = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool AfterStart();
    virtual void OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
protected:

  protected:
    INetModule *m_pNetModule;
    ILogModule *m_pLogModule;
  private:
};

}