#pragma once

#include <squick/core/base.h>

class Tutorial5 : public IPlugin
{
public:
	Tutorial5(IPluginManager* p)
    {
        pPluginManager = p;
    }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};