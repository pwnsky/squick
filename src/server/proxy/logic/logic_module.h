#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_logic_module.h"

namespace proxy::logic {
class LogicModule : public ILogicModule {
  public:
    LogicModule(IPluginManager *p) { pPluginManager = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

  protected:
    void OnOtherMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);
    void OnHeartbeat(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqConnect(const SQUICK_SOCKET sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqEnterGameServer(const SQUICK_SOCKET sock, const int msg_id, const char* msg, const uint32_t len);
    bool SelectGameServer(int sock);

    int EnterGameSuccessEvent(const Guid xClientID, const Guid xPlayerID);

    virtual void OnClientConnected(const SQUICK_SOCKET nAddress) override;
    virtual void OnClientDisconnect(const SQUICK_SOCKET nAddress) override;
    
    int Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len);

    MapEx<Guid, SQUICK_SOCKET> mxClientIdent;
  protected:
    ILogModule* m_pLogModule;
    IClassModule *m_pClassModule;
    IKernelModule *m_pKernelModule;
    INetModule *m_pNetModule;
    INetClientModule *m_pNetClientModule;

  private:
};

}