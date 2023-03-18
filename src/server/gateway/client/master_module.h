#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include <squick/core/i_module.h>
#include <squick/plugin/net/i_net_client_module.h>

namespace gateway::client {

class IToMasterModule : public IModule {
  public:
    virtual INetClientModule *GetClusterModule() = 0;
};

class ToMasterModule : public IToMasterModule {
  public:
    ToMasterModule(IPluginManager *p) {
        m_bIsUpdate = true;
        pPluginManager = p;
        mLastReportTime = 0;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();
    virtual bool BeforeDestory();

    virtual void LogReceive(const char *str) {}
    virtual void LogSend(const char *str) {}

    virtual INetClientModule *GetClusterModule();

  protected:
    void OnSocketMSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet);

  protected:
    //////////////////////////////////////////////////////////////////////////
    void OnSelectServerResultProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnWorldInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    //////////////////////////////////////////////////////////////////////////
    void Register(INet *pNet);
    void ServerReport();

  private:
    INT64 mLastReportTime;
    IElementModule *m_pElementModule;
    IKernelModule *m_pKernelModule;
    IClassModule *m_pClassModule;
    ILogModule *m_pLogModule;
    INetClientModule *m_pNetClientModule;
};

} // namespace gateway::client