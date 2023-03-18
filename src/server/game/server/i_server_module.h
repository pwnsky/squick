#pragma once

#include <squick/core/i_module.h>
#include <squick/plugin/net/i_net_module.h>

class IGameServerNet_ServerModule : public IModule {
  public:
    struct GateBaseInfo {
        enum ERoleStatus {
            E_LOADING = 1,
            E_LOADED = 2,
        };

        GateBaseInfo() {
            nActorID = 0;
            gateID = 0;
            eStatus = E_LOADING;
        }

        GateBaseInfo(const int gateID, const Guid xIdent) {
            this->nActorID = 0;
            this->gateID = gateID;
            this->xClientID = xIdent;
        }

        int nActorID;
        int gateID;
        ERoleStatus eStatus;
        Guid xClientID;
    };

    struct GateServerInfo {
        ServerData xServerData;

        std::map<Guid, SQUICK_SOCKET> xRoleInfo;
    };

  public:
    // virtual void SendMsgPBToGate(const uint16_t msgID, const std::string& xMsg, const Guid& self) = 0;
    virtual void SendMsgPBToGate(const uint16_t msgID, google::protobuf::Message &xMsg, const Guid &self) = 0;
    virtual void SendGroupMsgPBToGate(const uint16_t msgID, google::protobuf::Message &xMsg, const int sceneID, const int groupID) = 0;
    virtual void SendGroupMsgPBToGate(const uint16_t msgID, google::protobuf::Message &xMsg, const int sceneID, const int groupID, const Guid exceptID) = 0;

    virtual void SendMsgToGate(const uint16_t msgID, const std::string &msg, const Guid &self) = 0;
    virtual void SendGroupMsgPBToGate(const uint16_t msgID, const std::string &msg, const int sceneID, const int groupID) = 0;
    virtual void SendGroupMsgPBToGate(const uint16_t msgID, const std::string &msg, const int sceneID, const int groupID, const Guid exceptID) = 0;

    virtual bool AddPlayerGateInfo(const Guid &roleID, const Guid &clientID, const int gateID) = 0;
    virtual bool RemovePlayerGateInfo(const Guid &roleID) = 0;

    virtual SQUICK_SHARE_PTR<GateBaseInfo> GetPlayerGateInfo(const Guid &roleID) = 0;
    virtual SQUICK_SHARE_PTR<GateServerInfo> GetGateServerInfo(const int gateID) = 0;
    virtual SQUICK_SHARE_PTR<GateServerInfo> GetGateServerInfoBySockIndex(const SQUICK_SOCKET sockIndex) = 0;

    // 发送消息到Pvp服务器，由Pvp Manager作为代理
    virtual void SendMsgPBToGameplay(const uint16_t msgID, google::protobuf::Message &xMsg, const Guid &self) = 0;
    virtual void SendMsgToGameplay(const uint16_t msgID, const std::string &msg, const Guid &self) = 0;

    // 发送消息到Pvp Manager服务器，内部实现是选择第一个注册的PVP Manager 服务器
    virtual void SendMsgToGameplayManager(const uint16_t msgID, const std::string &msg) = 0;
    virtual void SendMsgPBToGameplayManager(const uint16_t msgID, google::protobuf::Message &xMsg) = 0;
};
