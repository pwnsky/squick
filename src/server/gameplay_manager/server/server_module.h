#pragma once

#include <map>
#include <iostream>
#include "squick/core/map.h"
#include <squick/struct/struct.h>
#include <squick/core/platform.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/lua/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/security/export.h>
#include "i_server_module.h"

namespace gameplay_manager::server {

class ServerModule
	: public IServerModule
{
public:
	ServerModule(IPluginManager* p)
	{
		pPluginManager = p;
        m_bIsUpdate = true;
	}

	virtual bool Start();
	virtual bool Destory();

	virtual bool AfterStart();
	virtual bool Update();

	virtual bool Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

protected:
	// PVP服务器连接处理
	void OnReqConnectGameServer(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
	void OnOtherMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len);

	// PVP服务器断开连接
	void OnClientDisconnect(const SQUICK_SOCKET nAddress);
	// 转发消息给PVP服务器
	
	
	// PVP服务器连接表
    MapEx<Guid, SQUICK_SOCKET> mxClientIdent;
protected:
    INetClientModule* m_pNetClientModule;
    IKernelModule* m_pKernelModule;
    ILogModule* m_pLogModule;
    IElementModule* m_pElementModule;
    IClassModule* m_pClassModule;
	INetModule* m_pNetModule;
    IWSModule* m_pWsModule;
	ISecurityModule* m_pSecurityModule;

};

}
