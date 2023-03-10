#pragma once

#include <squick/plugin/kernel/i_kernel_module.h>

#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/net/i_net_module.h>

#include "../server/i_server_module.h"
#include "../logic/i_logic_module.h"
#include "i_master_module.h"


class LoginToMasterModule
    : public ILoginToMasterModule
{
public:
    LoginToMasterModule(IPluginManager* p)
    {
        m_bIsUpdate = true;
        pPluginManager = p;
		mLastReportTime = 0;
    }


    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();
    virtual bool BeforeDestory();

    virtual void LogReceive(const char* str) {}
    virtual void LogSend(const char* str) {}

	virtual INetClientModule* GetClusterModule();
    virtual MapEx<int, SquickStruct::ServerInfoReport>& GetWorldMap();

protected:
    void OnSocketMSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet);

protected:

    //////////////////////////////////////////////////////////////////////////
	void OnSelectServerResultProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
	void OnWorldInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

    //////////////////////////////////////////////////////////////////////////
    void Register(INet* pNet);
	void ServerReport();

private:
	INT64 mLastReportTime;
    MapEx<int, SquickStruct::ServerInfoReport> mWorldMap;

    ILoginNet_ServerModule* m_pLoginNet_ServerModule;
    IElementModule* m_pElementModule;
    IKernelModule* m_pKernelModule;
    IClassModule* m_pClassModule;
    ILogModule* m_pLogModule;
	INetClientModule* m_pNetClientModule;
};
