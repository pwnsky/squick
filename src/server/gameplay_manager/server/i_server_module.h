#pragma once
#include <squick/core/base.h>

namespace gameplay_manager::server {
class IServerModule : public IModule {
  public:
    virtual bool Transport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) = 0;
};

} // namespace gameplay_manager::server