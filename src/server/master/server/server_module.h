#pragma once

#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include "i_server_module.h"

class MasterNet_ServerModule : public IMasterNet_ServerModule {
  public:
    MasterNet_ServerModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
        mnLastLogTime = pm_->GetNowTime();
    }
    virtual ~MasterNet_ServerModule();

    virtual bool Start();
    virtual bool Destory();

    virtual bool AfterStart();
    virtual bool Update();

    virtual void LogReceive(const char *str) {}
    virtual void LogSend(const char *str) {}

    virtual std::string GetServersStatus();

  protected:
    void OnSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);
    void OnClientDisconnect(const socket_t sock);
    void OnClientConnected(const socket_t sock);

  protected:
    void OnWorldRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnRefreshWorldInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    //////////////////////////////////////////////////////////////////////////
    void OnLoginRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnLoginUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnRefreshLoginInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnSelectWorldProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnSelectServerResultProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnServerReport(const socket_t nFd, const int msg_id, const char *buffer, const uint32_t len);

    //////////////////////////////////////////////////////////////////////////

    void SyncWorldToLoginAndWorld();
    void SyncProxyToLogin();
    void LogGameServer();

    void OnHeartBeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    INT64 mnLastLogTime;

    // serverid,data
    MapEx<int, ServerData> mMasterMap;
    MapEx<int, ServerData> mLoginMap;
    MapEx<int, ServerData> mWorldMap;
    MapEx<int, ServerData> mProxyMap;
    MapEx<int, ServerData> mGameMap;
    MapEx<int, ServerData> mGameplayManagerMap;

    IElementModule *m_element_;
    IClassModule *m_class_;
    IKernelModule *m_kernel_;
    ILogModule *m_log_;
    INetModule *m_net_;
};