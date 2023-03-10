#pragma once

#include <iostream>
#include <squick/core/i_module.h>

class IMasterNet_ServerModule
    : public IModule
{

public:
	virtual std::string GetServersStatus() = 0;
};
