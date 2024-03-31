#pragma once

#include <iostream>
#include <squick/imodule/i_node_module.h>

namespace proxy::node {
class INodeModule : public INodeBaseModule {
  public:
	  virtual bool OnReqProxyConnectVerify(INT64 session, const std::string& guid, const std::string& key) = 0;
	  //virtual int GetLoadBanlanceNode(ServerType type) override = 0 ;
};
} // namespace proxy::server