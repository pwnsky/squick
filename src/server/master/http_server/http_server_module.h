#pragma once

#include <squick/struct/struct.h>
#include <squick/core/platform.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/net/i_http_server_module.h>

#include "i_http_server_module.h"
#include "../server/i_server_module.h"
#include "../logic/i_master_module.h"

class MasterNet_HttpServerModule
	: public IMasterNet_HttpServerModule
{
public:
	MasterNet_HttpServerModule(IPluginManager* p)
	{
        m_bIsUpdate = true;
		pPluginManager = p;
	}

	virtual bool Start();
	virtual bool Destory();

	virtual bool AfterStart();
	virtual bool Update();

protected:
	bool OnCommandQuery(SQUICK_SHARE_PTR<HttpRequest> req);

	WebStatus OnFilter(SQUICK_SHARE_PTR<HttpRequest> req);

private:
	IKernelModule* m_pKernelModule;
	IHttpServerModule* m_pHttpNetModule;
	IMasterNet_ServerModule* m_pMasterServerModule;
	IClassModule* m_pLogicClassModule;
	IElementModule* m_pElementModule;
};