#include "plugin.h"

#include "node_module.h"
namespace proxy::node {

bool NodeModule::AfterStart() {
    m_logic_ = pm_->FindModule<logic::ILogicModule>();
    Listen();

    // Lobby
    m_net_client_->AddReceiveCallBack(ServerType::ST_GAME, rpc::LobbyBaseRPC::ACK_ENTER, this, &NodeModule::OnAckEnter);
    m_net_client_->AddReceiveCallBack(ServerType::ST_GAME, this, &NodeModule::Transport);
    

    // Game

    
    // Gameplay Manager


    // Micro


    // Wrold


    // Login
    m_net_client_->AddReceiveCallBack(ServerType::ST_LOGIN, rpc::LoginRPC::ACK_PROXY_CONNECT_VERIFY, this, &NodeModule::OnAckProxyConnectVerify);


    AddServer(ServerType::ST_GAME);
    AddServer(ServerType::ST_GAMEPLAY_MANAGER);
    AddServer(ServerType::ST_LOBBY);
    AddServer(ServerType::ST_WORLD);
    AddServer(ServerType::ST_LOBBY);
    //AddServer(ServerType::ST_MICRO);
    return true;
}

void NodeModule::Transport(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    m_logic_->ForwardToClient(sock, msg_id, msg, len);
}

void NodeModule::OnAckEnter(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    dout << "进入游戏成功!\n";
    Guid nPlayerID;
    rpc::AckEnter xData;
    if (!INetModule::ReceivePB(msg_id, msg, len, xData, nPlayerID)) {
        return;
    }
    const Guid& xClient = INetModule::ProtobufToStruct(xData.guid());
    const Guid& xPlayer = INetModule::ProtobufToStruct(xData.object());

    // m_logic_->EnterGameSuccessEvent(xClient, xPlayer);
    m_logic_->ForwardToClient(sock, msg_id, msg, len);
}

bool NodeModule::OnReqProxyConnectVerify(INT64 session, const std::string& guid, const std::string& key) {
    // dout << "请求验证\n";
    rpc::ReqConnectProxyVerify req;
    req.set_session(session);
    req.set_key(key);
    req.set_guid(guid);
    m_net_client_->SendToServerByPB(2, rpc::LoginRPC::REQ_PROXY_CONNECT_VERIFY, req); // 暂时写login_id 为死的ID
    return true;
}

void NodeModule::OnAckProxyConnectVerify(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    m_logic_->OnAckConnectVerify(msg_id, msg, len);
    return;
}

bool NodeModule::Destory() { return true; }

void NodeModule::OnClientDisconnect(socket_t sock) { m_logic_->OnClientDisconnect(sock); }
void NodeModule::OnClientConnected(socket_t sock) { m_logic_->OnClientConnected(sock); }

} // namespace proxy::server