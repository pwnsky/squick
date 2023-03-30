#pragma once

#include <squick/struct/struct.h>

// #include "../server/world_logic.h"
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/plugin/security/i_security_module.h>

#include "../server/i_server_module.h"
#include "i_master_module.h"

class WorldToMasterModule : public IWorldToMasterModule {
  public:
    WorldToMasterModule(IPluginManager *p) {
        pm_ = p;
        mLastReportTime = 0;
        is_update_ = true;
    }

    virtual bool Start();
    virtual bool BeforeDestory();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

  protected:
    void OnSocketMSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

    void OnClientDisconnect(const socket_t sock);

    void OnClientConnected(const socket_t sock);

    virtual void LogServerInfo(const std::string &strServerInfo);

    void Register(INet *pNet);
    void ServerReport();
    void RefreshWorldInfo();
    void OnServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnSelectServerProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnKickClientProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    INT64 mLastReportTime;

    ILogModule *m_log_;
    IElementModule *m_element_;
    IClassModule *m_class_;
    IWorldNet_ServerModule *m_pWorldNet_ServerModule;
    INetClientModule *m_net_client_;
    INetModule *m_net_;
    ISecurityModule *m_security_;
};
