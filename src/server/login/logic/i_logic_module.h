#pragma once

#include <squick/plugin/net/i_net_module.h>

namespace login::logic {

class ILogicModule : public IModule {
  public:
    virtual void OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) = 0;
};

} // namespace login::logic