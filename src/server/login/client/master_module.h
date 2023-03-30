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
        is_update_ = true;
        pm_ = p;
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
    virtual map<int, rpc::ServerInfoReport> & GetWorldServers();
    virtual map<int, rpc::ServerInfoReport>& GetProxyServers();

  protected:
    void OnSocketMSEvent(const socket_t sock, const SQUICK_NET_EVENT event, INet *net);

  protected:
    //////////////////////////////////////////////////////////////////////////
    void OnSelectServerResultProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    //////////////////////////////////////////////////////////////////////////
    void Register(INet *pNet);
    void ServerReport();

  private:
    INT64 mLastReportTime;
    map<int, rpc::ServerInfoReport> world_servers_;
    map<int, rpc::ServerInfoReport> proxy_servers_;

    server::IServerModule *m_pLoginNet_ServerModule;
    IElementModule *m_element_;
    IKernelModule *m_kernel_;
    IClassModule *m_class_;
    ILogModule *m_log_;
    INetClientModule *m_net_client_;
};

} // namespace login::client