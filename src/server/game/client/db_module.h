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
        pm_ = p;
        mLastReportTime = 0;
    }
    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

    virtual void TransmitToDB(const int nHashKey, const int msg_id, const google::protobuf::Message &xData);

  protected:
    void OnSocketWSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

  protected:
    void TransPBToProxy(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    INT64 mLastReportTime;

    ILogModule *m_log_;
    IKernelModule *m_kernel_;
    IClassModule *m_class_;
    IElementModule *m_element_;
    INetClientModule *m_net_client_;
    IGameServerNet_ServerModule *m_pGameServerNet_ServerModule;
};