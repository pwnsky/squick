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
        pPluginManager = p;
        mLastReportTime = 0;
        m_bIsUpdate = true;
    }

    virtual bool Start();
    virtual bool BeforeDestory();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

  protected:
    void OnSocketMSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet);

    void OnClientDisconnect(const SQUICK_SOCKET nAddress);

    void OnClientConnected(const SQUICK_SOCKET nAddress);

    virtual void LogServerInfo(const std::string &strServerInfo);

    void Register(INet *pNet);
    void ServerReport();
    void RefreshWorldInfo();
    void OnServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    void OnSelectServerProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnKickClientProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    void InvalidMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

  private:
    INT64 mLastReportTime;

    ILogModule *m_pLogModule;
    IElementModule *m_pElementModule;
    IClassModule *m_pClassModule;
    IWorldNet_ServerModule *m_pWorldNet_ServerModule;
    INetClientModule *m_pNetClientModule;
    INetModule *m_pNetModule;
    ISecurityModule *m_pSecurityModule;
};
