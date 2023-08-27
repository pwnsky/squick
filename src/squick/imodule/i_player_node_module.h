// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-07-28
// Description: Player node module
#pragma once
#include "i_node_module.h"

class IPlayerNodeModule : public INodeBaseModule {
public:
    virtual bool Listen() override {
        
        auto ret = INodeBaseModule::Listen();
        // Player action
        m_net_->AddReceiveCallBack(rpc::ServerRPC::PLAYER_ENETER, this, &IPlayerNodeModule::OnPlayerEnter);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::PLAYER_LEAVE, this, &IPlayerNodeModule::OnPlayerLeave);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::PLAYER_OFFLINE, this, &IPlayerNodeModule::OnPlayerOffline);
        return ret;
    }

    // 发送消息给玩家
    virtual void SendToPlayer(const uint16_t msg_id, google::protobuf::Message& msg, const Guid& player) {}

    virtual void SendToPlayer(const uint16_t msg_id, const std::string& msg, const Guid& self) {}


    // 发送给玩家所连接相应类型的服务器
    virtual void SendToNode(const uint16_t msg_id, google::protobuf::Message& msg, const Guid& player, ServerType type) {
    }

    virtual void SendToNode(const uint16_t msg_id, const std::string& msg, const Guid& player, ServerType type) {
    }

    virtual void OnPlayerEnter(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    }

    virtual void OnPlayerLeave(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {}

    virtual void OnPlayerOffline(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {}
	struct PlayerProxyInfo {
		int proxy_id;
		int proxy_sock;
	};
public:
	// 玩家表
	map<Guid, PlayerProxyInfo> players_;
};
