#pragma once

#include <map>
#include <iostream>

#include <squick/struct/struct.h>
#include <squick/core/platform.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/lua/export.h>

namespace gateway::lua {
class ILuaBindModule : public IModule
{

};

class LuaBindModule
	: public ILuaBindModule
{
public:
	LuaBindModule(IPluginManager* p)
	{
		pPluginManager = p;
        m_bIsUpdate = true;
	}

	virtual bool Start();
	virtual bool Destory();

	virtual bool AfterStart();
	virtual bool Update();

protected:
	bool OnGetServerList(SQUICK_SHARE_PTR<HttpRequest> req);
	bool Bind();

private:
	IKernelModule* m_pKernelModule;
	IHttpServerModule* m_pHttpNetModule;
	IClassModule* m_pLogicClassModule;
	IElementModule* m_pElementModule;
	ILuaScriptModule* m_pLuaScriptModule;
};

}
