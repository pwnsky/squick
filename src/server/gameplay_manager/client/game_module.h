#pragma once

#include <string>
#include <squick/struct/struct.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/log/i_log_module.h>

#include <squick/plugin/net/i_net_client_module.h>

#include "../server/i_server_module.h"
#include "../logic/i_logic_module.h"

namespace gameplay_manager::client {
class IGameModule
    : public  IModule
{
public:
};

class GameModule : public IGameModule
{
public:

    GameModule(IPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

protected:

    void OnSocketGSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet);

    void Register(INet* pNet);

    void LogServerInfo(const std::string& strServerInfo);

	void Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

private:

    ILogModule* m_pLogModule;
    IKernelModule* m_pKernelModule;
    server::IServerModule* m_ServerModule;
    IElementModule* m_pElementModule;
    IClassModule* m_pClassModule;
	INetClientModule* m_pNetClientModule;

};

}