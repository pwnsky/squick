#pragma once

#include <squick/core/base.h>

namespace login::mysql {

class IMysqlModule : public IModule {
  public:
    virtual void OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) = 0;
    
};

} // namespace login::logic