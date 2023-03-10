#pragma once

#include <squick/core/i_plugin.h>
#include <squick/core/i_plugin_manager.h>


//////////////////////////////////////////////////////////////////////////
class LoginNet_HttpServerPlugin : public IPlugin
{
public:
	LoginNet_HttpServerPlugin(IPluginManager* p)
	{
		pPluginManager = p;
	}
	virtual const int GetPluginVersion();

	virtual const std::string GetPluginName();

	virtual void Install();

	virtual void Uninstall();
};

