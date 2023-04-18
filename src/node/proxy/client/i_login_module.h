#pragma once
#include <server/i_node_client_module.h>
#include <squick/core/base.h>
#include <string>
namespace proxy::client {
class ILoginModule : public INodeClientModule {
  public:
    virtual bool OnReqProxyConnectVerify(INT64 session, const std::string &guid, const std::string &key) = 0;
};

} // namespace proxy::client