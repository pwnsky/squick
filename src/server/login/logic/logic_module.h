#pragma once

#include "squick/struct/struct.h"
#include "squick/core/map.h"
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_logic_module.h"


namespace login::logic {
class LogicModule
    : public ILogicModule
{
public:
    LogicModule(IPluginManager* p)
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

}