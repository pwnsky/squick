#pragma once

#include <squick/plugin/kernel/export.h>

#include <squick/plugin/config/export.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/export.h>

#include "../logic/i_logic_module.h"
#include "../server/i_server_module.h"
#include "i_master_module.h"
#include <map>
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
    virtual map<int, SquickStruct::ServerInfoReport> & GetWorldServers();
    virtual map<int, SquickStruct::ServerInfoReport>& GetProxyServers();

  protected:
    void OnSocketMSEvent(const SQUICK_SOCKET sock, const SQUICK_NET_EVENT event, INet *net);

  protected:
    //////////////////////////////////////////////////////////////////////////
    void OnSelectServerResultProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnWorldInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    //////////////////////////////////////////////////////////////////////////
    void Register(INet *pNet);
    void ServerReport();

  private:
    INT64 mLastReportTime;
    map<int, SquickStruct::ServerInfoReport> world_servers_;
    map<int, SquickStruct::ServerInfoReport> proxy_servers_;

    server::IServerModule *m_pLoginNet_ServerModule;
    IElementModule *m_pElementModule;
    IKernelModule *m_pKernelModule;
    IClassModule *m_pClassModule;
    ILogModule *m_pLogModule;
    INetClientModule *m_pNetClientModule;
};

} // namespace login::client