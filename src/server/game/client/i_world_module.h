#pragma once

#include <iostream>
#include <squick/core/i_module.h>
#include <squick/plugin/net/i_net_module.h>

class IGameServerToWorldModule
	: public IModule
{
public:
	virtual void TransmitToWorld(const int nHashKey, const int msgID, const google::protobuf::Message& xData) = 0;
	virtual void SendOnline(const Guid& self) = 0;
	virtual void SendOffline(const Guid& self) = 0;
};
