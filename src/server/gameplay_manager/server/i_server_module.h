#pragma once
#include <squick/core/base.h>

namespace gameplay_manager::server {
class IServerModule : public IModule
{
public:
    virtual bool Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len) = 0;
};

}