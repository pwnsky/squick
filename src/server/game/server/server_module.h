#pragma once

#include <memory>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include <squick/plugin/kernel/i_event_module.h>
#include <squick/plugin/kernel/i_scene_module.h>
#include <squick/plugin/kernel/i_schedule_module.h>
#include <squick/plugin/kernel/i_thread_pool_module.h>

#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>

#include "../client/i_world_module.h"
#include "i_server_module.h"
#include <squick/core/i_module.h>
////////////////////////////////////////////////////////////////////////////

namespace game::server {
class GameServerNet_ServerModule : public IGameServerNet_ServerModule {
  public:
    GameServerNet_ServerModule(IPluginManager *p) { pm_ = p; }
    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

    virtual void SendMsgPBToProxy(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &self);
    virtual void SendGroupMsgPBToProxy(const uint16_t msg_id, google::protobuf::Message &xMsg, const int sceneID, const int groupID);
    virtual void SendGroupMsgPBToProxy(const uint16_t msg_id, google::protobuf::Message &xMsg, const int sceneID, const int groupID, const Guid exceptID);

    virtual void SendMsgToProxy(const uint16_t msg_id, const std::string &msg, const Guid &self);
    virtual void SendGroupMsgPBToProxy(const uint16_t msg_id, const std::string &msg, const int sceneID, const int groupID);
    virtual void SendGroupMsgPBToProxy(const uint16_t msg_id, const std::string &msg, const int sceneID, const int groupID, const Guid exceptID);

    // 发送消息到服务器，由 Manager作为代理
    void SendMsgPBToGameplay(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &self) override;
    void SendMsgToGameplay(const uint16_t msg_id, const std::string &msg, const Guid &self) override;

    // 发送消息到 Manager服务器，内部实现是选择第一个注册的PVP Manager 服务器
    void SendMsgToGameplayManager(const uint16_t msg_id, const std::string &msg) override;
    void SendMsgPBToGameplayManager(const uint16_t msg_id, google::protobuf::Message &xMsg) override;

    virtual bool AddPlayerProxyInfo(const Guid &roleID, const Guid &clientID, const int proxy_id);
    // virtual bool AddProxyInfo(const Guid& clientID, const int gateID);

    virtual bool RemovePlayerProxyInfo(const Guid &roleID);
    virtual std::shared_ptr<ProxyBaseInfo> GetPlayerProxyInfo(const Guid &roleID) override;
    virtual std::shared_ptr<ProxyServerInfo> GetProxyServerInfo(const int proxy_id) override;
    virtual std::shared_ptr<ProxyServerInfo> GetProxyServerInfoBySockIndex(const socket_t sock) override;

  protected:
    void OnSocketPSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);
    void OnClientDisconnect(const socket_t sock);
    void OnClientConnected(const socket_t sock);

  protected:
    // 代理服务器注册
    void OnProxyServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnProxyServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnRefreshProxyServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    // PVP管理服务器注册
    void OnPvpManagerServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnPvpManagerServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnRefreshPvpManagerServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  protected:
    void OnLagTestProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    ///////////WORLD_START///////////////////////////////////////////////////////////////
    void OnTransWorld(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    MapEx<Guid, ProxyBaseInfo> mRoleBaseData;

    // gateid,data
    MapEx<int, ProxyServerInfo> mProxyMap;

    // PVP Manager 服务器连接表
    MapEx<int, ProxyServerInfo> mGameplayManagerMap;
    //////////////////////////////////////////////////////////////////////////
    IKernelModule *m_kernel_;
    IClassModule *m_class_;
    ILogModule *m_log_;
    IElementModule *m_element_;
    INetModule *m_net_;
    IEventModule *m_event_;
    ISceneModule *m_scene_;
    INetClientModule *m_net_client_;
    IScheduleModule *m_schedule_;
    IThreadPoolModule *m_thread_pool_;
};

} // namespace game::server