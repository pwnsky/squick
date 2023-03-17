#pragma once
#include "squick/core/map.h"

#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/config/export.h>
#include <squick/core/base.h>

#include "i_server_module.h"
#include "../logic/i_logic_module.h"
#include "../client/i_master_module.h"

#include "squick/struct/struct.h"

namespace login::server {
class ServerModule
    : public IServerModule {
public:
    ServerModule(IPluginManager* p) {
        pPluginManager = p;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool BeforeDestory();
    virtual bool AfterStart();

    virtual void LogReceive(const char* str) {}
    virtual void LogSend(const char* str) {}

protected:
    void OnSocketClientEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet);

protected:
    void OnClientDisconnect(const SQUICK_SOCKET nAddress);
    void OnClientConnected(const SQUICK_SOCKET nAddress);
    void OnSelectWorldProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
	void InvalidMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

protected:

protected:
    void SynWorldToClient(const SQUICK_SOCKET nFD);

    MapEx<Guid, SQUICK_SOCKET> mxClientIdent;

private:

	INetModule* m_pNetModule;
	INetClientModule* m_pNetClientModule;
    IClassModule* m_pClassModule;
    IElementModule* m_pElementModule;
    IKernelModule* m_pKernelModule;
    ILogModule* m_pLogModule;
	client::IMasterModule* m_pLoginToMasterModule;
    IThreadPoolModule* m_pThreadPoolModule;
};

}