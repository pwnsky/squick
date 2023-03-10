#pragma once

#include <squick/plugin/net/i_net_module.h>

class ILoginLogicModule
    : public IModule
{
public:
    virtual void OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len) = 0;

};