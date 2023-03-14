#pragma once

#include "squick/struct/struct.h"
#include "squick/core/map.h"
#include <squick/plugin/kernel/i_kernel_module.h>
//#include "squick/base/login_net_server.h"
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>

#include "i_logic_module.h"
//#include <server/db/logic/i_account_redis_module.h>


class LoginLogicModule
    : public ILoginLogicModule
{
public:
    LoginLogicModule(IPluginManager* p)
    {
        pPluginManager = p;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool ReadyUpdate();
    virtual bool Update();

    virtual bool AfterStart();

    virtual void OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

protected:
	INetModule* m_pNetModule;
	ILogModule* m_pLogModule;
	//IAccountRedisModule* m_pAccountRedisModule;
private:
};