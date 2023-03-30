#pragma once

#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include "../server/i_server_module.h"
#include "i_world_module.h"
class GameServerToWorldModule : public IGameServerToWorldModule {
  public:
    GameServerToWorldModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
        mLastReportTime = 0;
    }
    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

    virtual void TransmitToWorld(const int nHashKey, const int msg_id, const google::protobuf::Message &xData);

    virtual void SendOnline(const Guid &self);
    virtual void SendOffline(const Guid &self);

  protected:
    void OnSocketWSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

  protected:
    void Register(INet *pNet);
    void ServerReport();
    void TransPBToProxy(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    int OnObjectClassEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var);

    void OnServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  protected:
    void OnWorldPropertyIntProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldPropertyFloatProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldPropertyStringProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldPropertyObjectProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldPropertyVector2Process(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldPropertyVector3Process(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnWorldRecordEnterProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldAddRowProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldRemoveRowProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldSwapRowProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldRecordIntProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldRecordFloatProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldRecordStringProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldRecordObjectProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldRecordVector2Process(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnWorldRecordVector3Process(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    INT64 mLastReportTime;

    ILogModule *m_log_;
    IKernelModule *m_kernel_;
    IClassModule *m_class_;
    IElementModule *m_element_;
    INetClientModule *m_net_client_;
    IGameServerNet_ServerModule *m_pGameServerNet_ServerModule;
};