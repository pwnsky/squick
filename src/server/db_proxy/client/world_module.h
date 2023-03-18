#ifndef SQUICK_DB_NET_CLIENT_MODULE_H
#define SQUICK_DB_NET_CLIENT_MODULE_H

#include "squick/struct/struct.h"

#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>

#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/security/i_security_module.h>

#include "i_world_module.h"
#include <server/world/client/i_master_module.h>
#include <server/world/server/i_server_module.h>

class DBToWorldModule : public IDBToWorldModule {
  public:
    DBToWorldModule(IPluginManager *p) {
        m_bIsUpdate = true;
        pPluginManager = p;
        mLastReportTime = 0;
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
    void OnServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    void InvalidMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

  private:
    INT64 mLastReportTime;

    ILogModule *m_pLogModule;
    IElementModule *m_pElementModule;
    IClassModule *m_pClassModule;
    INetClientModule *m_pNetClientModule;
    INetModule *m_pNetModule;
    ISecurityModule *m_pSecurityModule;
};

#endif