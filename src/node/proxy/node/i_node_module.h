#pragma once

#include <iostream>
#include <squick/imodule/i_node_base_module.h>

namespace proxy::node {
class INodeModule : public INodeBaseModule {
  public:
	  virtual bool OnReqProxyConnectVerify(INT64 session, const std::string& guid, const std::string& key) = 0;
};
} // namespace proxy::server