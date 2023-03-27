#pragma once

#include <squick/plugin/kernel/export.h>

#include <squick/plugin/config/export.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/export.h>

#include "../logic/i_logic_module.h"
#include "../server/i_server_module.h"
#include "i_master_module.h"

namespace login::client {
class MasterModule : public IMasterModule {
  public:
    MasterModule(IPluginManager *p) {
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
    virtual MapEx<int, SquickStruct::ServerInfoReport> &GetWorldMap();

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
    MapEx<int, SquickStruct::ServerInfoReport> mWorldMap;
    MapEx<int, SquickStruct::ServerInfoReport> proxys_map_;

    server::IServerModule *m_pLoginNet_ServerModule;
    IElementModule *m_pElementModule;
    IKernelModule *m_pKernelModule;
    IClassModule *m_pClassModule;
    ILogModule *m_pLogModule;
    INetClientModule *m_pNetClientModule;
};

} // namespace login::client