#pragma once
#include <squick/core/base.h>
#include <string>

namespace proxy::client {
class ILoginModule : public IModule {
  public:
    virtual bool OnReqProxyConnectVerify(const std::string &guid, const std::string &key) = 0;
};

} // namespace proxy::client