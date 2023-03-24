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
        m_bIsUpdate = true;
        pPluginManager = p;
        mnLastLogTime = pPluginManager->GetNowTime();
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
    void OnSocketEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet);
    void OnClientDisconnect(const SQUICK_SOCKET nAddress);
    void OnClientConnected(const SQUICK_SOCKET nAddress);

  protected:
    void OnWorldRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnWorldUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnRefreshWorldInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    //////////////////////////////////////////////////////////////////////////
    void OnLoginRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnLoginUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnRefreshLoginInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    void OnSelectWorldProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnSelectServerResultProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    void OnServerReport(const SQUICK_SOCKET nFd, const int msgId, const char *buffer, const uint32_t len);

    //////////////////////////////////////////////////////////////////////////

    void SynWorldToLoginAndWorld();
    void LogGameServer();

    void OnHeartBeat(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void InvalidMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

  private:
    INT64 mnLastLogTime;

    // serverid,data
    MapEx<int, ServerData> mMasterMap;
    MapEx<int, ServerData> mLoginMap;
    MapEx<int, ServerData> mWorldMap;
    MapEx<int, ServerData> mProxyMap;
    MapEx<int, ServerData> mGameMap;
    MapEx<int, ServerData> mGameplayManagerMap;

    IElementModule *m_pElementModule;
    IClassModule *m_pClassModule;
    IKernelModule *m_pKernelModule;
    ILogModule *m_pLogModule;
    INetModule *m_pNetModule;
};