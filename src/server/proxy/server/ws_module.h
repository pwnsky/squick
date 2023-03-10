#pragma once

#include <squick/core/consistent_hash.h>
#include <squick/struct/struct.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_ws_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/security/i_security_module.h>

#include "i_ws_module.h"
#include "../client/i_world_module.h"
#include "../client/i_game_module.h"
namespace proxy::server {
class ProxyServerNet_WSModule : public IWS_Module
{
public:
    ProxyServerNet_WSModule(IPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

protected:

    void OnSocketClientEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet);

    void OnClientDisconnect(const SQUICK_SOCKET nAddress);
    void OnClientConnected(const SQUICK_SOCKET nAddress);

    void OnWebSocketTestProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

protected:

    MapEx<Guid, SQUICK_SOCKET> mxClientIdent;
protected:
    INetClientModule* m_pNetClientModule;
    IKernelModule* m_pKernelModule;
    ILogModule* m_pLogModule;
    IElementModule* m_pElementModule;
    IClassModule* m_pClassModule;
	IWSModule* m_pWSModule;
	ISecurityModule* m_pSecurityModule;
	client::IWorldModule* m_pProxyToWorldModule;
};

}