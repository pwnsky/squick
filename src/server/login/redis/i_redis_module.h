#pragma once

#include <squick/core/base.h>

namespace login::redis {

class IRedisModule : public IModule {
  public:
    virtual bool GenerateProxyConnectKey(const Guid &guid, int proxy_id, const string &key) = 0;
};

}