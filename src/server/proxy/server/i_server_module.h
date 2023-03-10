#pragma once

#include <iostream>
#include <squick/core/i_module.h>
#include <squick/plugin/net/i_net_client_module.h>

namespace proxy::server {
class IServerModule
    :  public IModule
{

public:
    virtual int Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len) = 0;
    virtual int EnterGameSuccessEvent(const Guid xClientID, const Guid xPlayerID) = 0;
};
}