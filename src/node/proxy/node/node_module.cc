#include "plugin.h"

#include "node_module.h"
namespace proxy::node {

bool NodeModule::AfterStart() {
    m_logic_ = pm_->FindModule<logic::ILogicModule>();
    Listen();

    // Lobby
    m_net_client_->AddReceiveCallBack(ServerType::ST_LOBBY, this, &NodeModule::Transport);
    m_net_client_->AddReceiveCallBack(ServerType::ST_LOGIN, rpc::LoginRPC::ACK_PROXY_CONNECT_VERIFY, this, &NodeModule::OnAckProxyConnectVerify);
    m_net_client_->AddReceiveCallBack(ServerType::ST_LOBBY, rpc::PlayerEventRPC::PLAYER_BIND_EVENT, this, &NodeModule::PlayerBindEvent);
    AddServer(ServerType::ST_LOBBY);
    AddServer(ServerType::ST_LOGIN);
    AddServer(ServerType::ST_WORLD);

    // ws
    
    return true;
}

void NodeModule::Transport(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    m_logic_->ForwardToClient(sock, msg_id, msg, len);
}

void NodeModule::PlayerBindEvent(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    string guid;
    rpc::PlayerBindEvent event;
    if (!INetModule::ReceivePB(msg_id, msg, len, event, guid)) {
        return;
    }
    m_logic_->EnterSuccessEvent(event.account_id(), event.player_id());
}

bool NodeModule::OnReqProxyConnectVerify(INT64 session, const std::string& guid, const std::string& key) {
    rpc::ReqConnectProxyVerify req;
    req.set_session(session);
    req.set_key(key);
    req.set_guid(guid);
    m_net_client_->SendToAllServerByPB(ServerType::ST_LOGIN, rpc::LoginRPC::REQ_PROXY_CONNECT_VERIFY, req, "");
    return true;
}

void NodeModule::OnAckProxyConnectVerify(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    m_logic_->OnAckConnectVerify(msg_id, msg, len);
    return;
}

bool NodeModule::Destory() { return true; }

void NodeModule::OnClientConnected(socket_t sock) {  }

void NodeModule::OnClientDisconnected(socket_t sock) { m_logic_->OnClientDisconnected(sock); }

} // namespace proxy::server