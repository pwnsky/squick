#pragma once
#include "squick/struct/struct.h"
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/net/i_net_module.h>

#include "../server/i_server_module.h"
#include "i_db_module.h"

class GameServerToDBModule : public IGameServerToDBModule {
  public:
    GameServerToDBModule(IPluginManager *p) {
        pPluginManager = p;
        mLastReportTime = 0;
    }
    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

    virtual void TransmitToDB(const int nHashKey, const int msgID, const google::protobuf::Message &xData);

  protected:
    void OnSocketWSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet);

  protected:
    void TransPBToProxy(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    INT64 mLastReportTime;

    ILogModule *m_pLogModule;
    IKernelModule *m_pKernelModule;
    IClassModule *m_pClassModule;
    IElementModule *m_pElementModule;
    INetClientModule *m_pNetClientModule;
    IGameServerNet_ServerModule *m_pGameServerNet_ServerModule;
};