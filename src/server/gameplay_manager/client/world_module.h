#pragma once

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include <squick/core/i_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/struct/struct.h>

namespace gameplay_manager::client {

class IWorldModule : public IModule {
  public:
    virtual INetClientModule *GetClusterModule() = 0;
};

class WorldModule : public IWorldModule {
  public:
    WorldModule(IPluginManager *p) {
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

  protected:
    void OnSocketMSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

  protected:
    //////////////////////////////////////////////////////////////////////////
    void OnSelectServerResultProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    // void OnWorldInfoProcess(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    //////////////////////////////////////////////////////////////////////////
    void Register(INet *pNet);
    void ServerReport();

  private:
    INT64 mLastReportTime;
    IElementModule *m_element_;
    IKernelModule *m_kernel_;
    IClassModule *m_class_;
    ILogModule *m_log_;
    INetClientModule *m_net_client_;
};

} // namespace gameplay_manager::client