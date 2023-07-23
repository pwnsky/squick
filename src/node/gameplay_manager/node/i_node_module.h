#pragma once
#include <squick/imodule/i_node_base_module.h>

namespace gameplay_manager::node {
class INodeModule : public INodeBaseModule {
  public:
    virtual bool Transport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) = 0;
};

} // namespace gameplay_manager::node