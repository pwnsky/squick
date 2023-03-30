#pragma once

#include <squick/core/i_module.h>
#include <squick/plugin/net/i_net_module.h>

class IGameServerNet_ServerModule : public IModule {
  public:
    struct ProxyBaseInfo {
        enum ERoleStatus {
            E_LOADING = 1,
            E_LOADED = 2,
        };

        ProxyBaseInfo() {
            actor_id_ = 0;
            proxy_id_ = 0;
            eStatus = E_LOADING;
        }

        ProxyBaseInfo(const int proxy_id, const Guid xIdent) {
            this->actor_id_ = 0;
            this->proxy_id_ = proxy_id;
            this->xClientID = xIdent;
        }

        int actor_id_;
        int proxy_id_;
        ERoleStatus eStatus;
        Guid xClientID;
    };

    struct ProxyServerInfo {
        ServerData xServerData;
        std::map<Guid, socket_t> xRoleInfo;
    };

  public:
    virtual void SendMsgPBToProxy(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &self) = 0;
    virtual void SendGroupMsgPBToProxy(const uint16_t msg_id, google::protobuf::Message &xMsg, const int sceneID, const int groupID) = 0;
    virtual void SendGroupMsgPBToProxy(const uint16_t msg_id, google::protobuf::Message &xMsg, const int sceneID, const int groupID, const Guid exceptID) = 0;

    virtual void SendMsgToProxy(const uint16_t msg_id, const std::string &msg, const Guid &self) = 0;
    virtual void SendGroupMsgPBToProxy(const uint16_t msg_id, const std::string &msg, const int sceneID, const int groupID) = 0;
    virtual void SendGroupMsgPBToProxy(const uint16_t msg_id, const std::string &msg, const int sceneID, const int groupID, const Guid exceptID) = 0;

    virtual bool AddPlayerProxyInfo(const Guid &roleID, const Guid &clientID, const int proxy_id) = 0;
    virtual bool RemovePlayerProxyInfo(const Guid &roleID) = 0;

    virtual std::shared_ptr<ProxyBaseInfo> GetPlayerProxyInfo(const Guid &roleID) = 0;
    virtual std::shared_ptr<ProxyServerInfo> GetProxyServerInfo(const int proxy_id) = 0;
    virtual std::shared_ptr<ProxyServerInfo> GetProxyServerInfoBySockIndex(const socket_t sock) = 0;

    // 发送消息到Pvp服务器，由Pvp Manager作为代理
    virtual void SendMsgPBToGameplay(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &self) = 0;
    virtual void SendMsgToGameplay(const uint16_t msg_id, const std::string &msg, const Guid &self) = 0;

    // 发送消息到Pvp Manager服务器，内部实现是选择第一个注册的PVP Manager 服务器
    virtual void SendMsgToGameplayManager(const uint16_t msg_id, const std::string &msg) = 0;
    virtual void SendMsgPBToGameplayManager(const uint16_t msg_id, google::protobuf::Message &xMsg) = 0;
};
