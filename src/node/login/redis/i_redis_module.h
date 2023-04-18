#pragma once

#include <squick/core/base.h>

namespace login::redis {

class IRedisModule : public IModule {
  public:
    virtual bool GenerateProxyConnectKey(const Guid &guid, int proxy_id, const string &key) = 0;
    virtual bool HashSet(const std::string &guid, const std::string &key, const std::string &value) = 0;
    virtual bool HashGet(const std::string &guid, const std::string &key, std::string &value) = 0;
};

} // namespace login::redis