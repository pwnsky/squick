
#include "logic_module.h"
#include "plugin.h"

namespace proxy::logic {

bool LogicModule::Start() { return true; }

bool LogicModule::Destory() { return true; }

bool LogicModule::Update() {
    return true;
}

bool LogicModule::AfterStart() {
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_schedule_ = pm_->FindModule<IScheduleModule>();
    m_node_ = pm_->FindModule<node::INodeModule>();
    m_ws_ = pm_->FindModule<IWSModule>();

    m_net_->AddReceiveCallBack(this, &LogicModule::OnOtherMessage);
    m_net_->AddReceiveCallBack(rpc::ProxyRPC::REQ_HEARTBEAT, this, &LogicModule::OnHeartbeat);
    m_net_->AddReceiveCallBack(rpc::ProxyRPC::REQ_CONNECT_PROXY, this, &LogicModule::OnReqConnectWithTcp);
    m_net_->AddReceiveCallBack(rpc::TestRPC::REQ_TEST_PROXY, this, &LogicModule::OnReqTestProxy);
    m_net_client_->AddReceiveCallBack(ServerType::ST_MASTER, this, &LogicModule::OnNnAckMinWorkloadNodeInfo);
    m_ws_->AddReceiveCallBack(rpc::ProxyRPC::REQ_CONNECT_PROXY, this, &LogicModule::OnReqConnectWithWS);
    m_ws_->AddReceiveCallBack(this, &LogicModule::OnOtherMessage);

    m_ws_->Startialization(DEFAULT_NODE_MAX_SERVER_CONNECTION, pm_->GetArg("ws_port=", 10502));
    m_ws_->AddEventCallBack(this, &LogicModule::OnWebSocketClientEvent);
    return true;
}

void LogicModule::OnWebSocketClientEvent(socket_t sock, const SQUICK_NET_EVENT eEvent, INet* pNet)
{
    if (eEvent & SQUICK_NET_EVENT_EOF)
    {
        m_log_->LogInfo(Guid(0, sock), "websocket NF_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
        OnClientDisconnected(sock);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR)
    {
        m_log_->LogInfo(Guid(0, sock), "websocket NF_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
        OnClientDisconnected(sock);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT)
    {
        m_log_->LogInfo(Guid(0, sock), "websocket NF_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
        OnClientDisconnected(sock);
    }else if (eEvent & SQUICK_NET_EVENT_CONNECTED)
    {
        m_log_->LogInfo(Guid(0, sock), "websocket NF_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
    }
}

// request per 5 sec
void LogicModule::NnReqMinWorkloadNodeInfo() {

}

void LogicModule::OnNnAckMinWorkloadNodeInfo(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {

}

void LogicModule::OnClientDisconnected(const socket_t sock) {
    dout << sock << " client disconnected\n";
    // remove session
    auto iter = sessions_.find(sock);
    if (iter != sessions_.end()) {
        sessions_.erase(iter);
    }
    
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (pNetObject) {
        if (pNetObject->GetAccountID().empty()) {
            return;
        }
        dout << "client disconnected " << pNetObject->GetAccountID() << std::endl;
        string player_id = pNetObject->GetPlayerID();
        string account_id = pNetObject->GetAccountID();
        auto iter = clients_.find(account_id);
        if (iter != clients_.end()) {
            // 判断是否有新的连接
            if (iter->second.sock == sock) {
                // 移除schedule
                m_schedule_->RemoveSchedule(iter->second.account_id);
                clients_.erase(iter);

                int nGameID = pNetObject->GetLobbyID();
                if (nGameID > 0) {

                    // when a net-object bind a account then tell that game-server
                    if (!pNetObject->GetPlayerID().empty()) {
                        rpc::PlayerLeaveEvent xData;
                        rpc::MsgBase xMsg;

                        xMsg.set_guid(player_id);
                        if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
                            return;
                        }

                        std::string msg;
                        if (!xMsg.SerializeToString(&msg)) {
                            return;
                        }

                        m_net_client_->SendByID(nGameID, rpc::PlayerEventRPC::PLAYER_LEAVE_EVENT, msg);

                        // 断开该掉连接
                        m_net_->GetNet()->CloseNetObject(sock);
                    }
                }
            }
        }
    }
}

// forward to client
int LogicModule::ForwardToClient(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << " ForwardToClient: " << std::endl;
    rpc::MsgBase xMsg;
    if (!xMsg.ParseFromArray(msg, len)) {
        char szData[MAX_PATH] = {0};
        sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
        return false;
    }

    // broadcast many palyers
    for (int i = 0; i < xMsg.broadcast_size(); ++i) {
        SendToPlayer(xMsg.broadcast(i), msg_id, xMsg.msg_data());
    }

    // send to one player
    if (xMsg.broadcast_size() <= 0) {
        SendToPlayer(xMsg.guid(), msg_id, xMsg.msg_data());
    }
    return true;
}

bool LogicModule::SendToPlayer(string player_id, const int msg_id, const string &data) {
    
    bool ret = false;
    try {
        string account_id = players_[player_id];
        auto& client = clients_[account_id];
        dout << " SendToPlayer: " << player_id << " msg_id: " << msg_id << " socks: " << client.sock << std::endl;
        if (client.protocol_type == ProtocolType::Tcp) {
            m_net_->SendMsgWithOutHead(msg_id, data, client.sock);
        }
        else if (client.protocol_type == ProtocolType::WS) {
            m_ws_->SendMsgWithOutHead(msg_id, data.data(), data.size(), client.sock);
        }
        
        ret = true;
    }
    catch (...) {
        ostringstream info;
        info << "Send to player failed, player_id: " << player_id;
        m_log_->LogError(info);
    }
    return ret;
}

int LogicModule::EnterSuccessEvent(const string account_id, const string player_id) {
    // 在这里创建玩家表

    auto iter = clients_.find(account_id);
    if (iter != clients_.end()) {
        NetObject* obj = nullptr;
        if (iter->second.protocol_type == ProtocolType::Tcp) {
            obj = m_net_->GetNet()->GetNetObject(iter->second.sock);
        }else if (iter->second.protocol_type == ProtocolType::WS) {
            obj = m_ws_->GetNet()->GetNetObject(iter->second.sock);
        }
        
        if (obj) {
            obj->SetPlayerID(player_id);
            players_[player_id] = account_id;
            obj->SetLobbyID(iter->second.lobby_id);
            iter->second.player_id = player_id;

            dout << " EnterSuccessEvent account_guid: " << account_id << " player_id: " << player_id << std::endl;
        }

        if (iter->second.protocol_type == ProtocolType::Tcp) {
            m_net_->SendMsgWithOutHead(rpc::PlayerRPC::ACK_PLAYER_ONLINE, "", obj->GetRealFD());
        }else if (iter->second.protocol_type == ProtocolType::WS) {
            m_ws_->SendMsgWithOutHead(rpc::PlayerRPC::ACK_PLAYER_ONLINE, "", 0,  obj->GetRealFD());
        }
        
    }
    return 0;
}

void LogicModule::OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    NetObject *obj = m_net_->GetNet()->GetNetObject(sock);
    if (!obj) {
        obj = m_ws_->GetNet()->GetNetObject(sock);
    }

    if (!obj) {
        return;
    }

    int lobby_id = obj->GetLobbyID();
    if (lobby_id <= 0) {
        return;
    }
    
    string player_id = obj->GetPlayerID();
    // check
    auto iter = players_.find(player_id);
    if (iter == players_.end()) {
        dout << "This player not exsisted\n";
        return;
    }

    rpc::MsgBase xMsg;
    if (!xMsg.ParseFromString(std::string(msg, len))) {
        char szData[MAX_PATH] = {0};
        sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
        m_log_->LogError(Guid(0, sock), szData, __FUNCTION__, __LINE__);
        return;
    }

    xMsg.set_guid(player_id);
    *xMsg.mutable_msg_data() = string(msg, len);
    std::string pak;
    if (!xMsg.SerializeToString(&pak)) {
        return;
    }
    // 根据ID 来转发至不同服务器
    if (msg_id >= 12000 && msg_id < 30000) {
        // 转发到Lobby
        m_net_client_->SendByID(lobby_id, msg_id, pak);
    } else if (msg_id >= 10000 && msg_id < 32000) {

    } else if (msg_id >= 30000 && msg_id < 32000) {

    } else if (msg_id >= 32000 && msg_id < 35000) {

    } else {
        // 不支持转发
    }
}

void LogicModule::OnHeartbeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    std::string msgData(msg, len);
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (pNetObject) {
        auto iter = clients_.find(pNetObject->GetAccountID());
        if (iter != clients_.end()) {
            iter->second.last_ping = SquickGetTimeMS();
            if (iter->second.protocol_type == ProtocolType::Tcp) {
                m_net_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_HEARTBEAT, msgData, sock);
            }
            else if(iter->second.protocol_type == ProtocolType::WS){
                m_ws_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_HEARTBEAT, msgData.data(), msgData.size(), sock);
            }
        }
    }
}

void LogicModule::OnReqTestProxy(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    INT64 now_time = SquickGetTimeMSEx();
    static INT64 last_time = 0;
    static INT64 request_time = 0;
    static INT64 last_request_times = 0;
    request_time++;

    
    if (now_time - last_time > 1000000) {
        rpc::Test req;
        string guid;
        INetModule::ReceivePB(msg_id, string(msg, len), req, guid);
        std::cout << "Proxy Test:\n" << "handle quests: " << request_time - last_request_times << " times/second \n req network time: " << (now_time - req.req_time()) / 1000.0f << " ms \n";
        last_time = now_time;
        last_request_times = request_time;
    }
    
    m_net_->SendMsgWithOutHead(rpc::TestRPC::ACK_TEST_PROXY, string(msg, len), sock);
}

int LogicModule::OnHeatbeatCheck(const Guid &self, const std::string &heartBeat, const float time, const int count) {
    auto iter = clients_.find(self.ToString());
    if (iter == clients_.end()) {
        dout << "No this player to heatbeat check\n";
        return 1;
    }
    // dout << "heatbeatcheck : " << self.ToString() << "  " << iter->second.last_ping << "  now " << SquickGetTimeMS() << std::endl;
    time_t now = SquickGetTimeMS();
    if (now - iter->second.last_ping > 30000) { // 大于30秒即断线
        OnClientDisconnected(iter->second.sock);
    }
    return 0;
}

// Enter game
bool LogicModule::TryEnter(string guid) {
    
    auto client = clients_.find(guid);
    if (client == clients_.end()) {
        return false;
    }
    //int lobby_id = m_node_->GetLoadBanlanceNode(ServerType::ST_LOBBY);
    int lobby_id = 0;
    if (lobby_id == -1) {
        return false;
    }
    rpc::PlayerEnterEvent event;
    event.set_account(client->second.account);
    event.set_account_id(client->second.account_id);
    event.set_ip(client->second.ip);
    event.set_proxy_id(pm_->GetAppID());
    m_net_client_->SendPBByID(lobby_id, rpc::PlayerEventRPC::PLAYER_ENTER_EVENT, event);
    client->second.lobby_id = lobby_id;
    return true;
}

Coroutine<bool> LogicModule::OnReqConnectWithTcp(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    OnReqConnect(ProtocolType::Tcp, sock, msg_id, msg, len);
    co_return;
}

void LogicModule::OnReqConnectWithWS(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    OnReqConnect(ProtocolType::WS, sock, msg_id, msg, len);
}

void LogicModule::OnReqConnect(ProtocolType type, const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    NetObject* pNetObject = nullptr;
    if (type == ProtocolType::Tcp) {
        pNetObject = m_net_->GetNet()->GetNetObject(sock);
    }
    else if (type == ProtocolType::WS) {
        pNetObject = m_ws_->GetNet()->GetNetObject(sock);
    }

    if (!pNetObject) {
        return;
    }


    rpc::ReqConnectProxy req;
    if (!req.ParseFromArray(msg, len)) {
        return;
    }
    // 验证Token
    Session s;
    s.account_id = req.account_id();
    s.key = req.key();
    s.time = SquickGetTimeMS();
    s.sock = sock;
    s.ip = pNetObject->GetIP();
    s.protocol_type = type;
    sessions_[sock] = s;
    m_node_->OnReqProxyConnectVerify(sock, req.account_id(), req.key());
}


void LogicModule::OnAckConnectVerify(const int msg_id, const char *msg, const uint32_t len) {
    string tmp;
    rpc::AckConnectProxyVerify data;
    if (!m_net_->ReceivePB(msg_id, msg, len, data, tmp)) {
        return;
    }

    // find session
    auto session = data.session();
    auto iter = sessions_.find(session);
    if (iter == sessions_.end()) {
        // session not exsist
        dout << " No this session: " << session << std::endl;
        return;
    }
    Session s = iter->second;
    // remove sessoin
    sessions_.erase(iter);

    NetObject* net = nullptr;
    if (s.protocol_type == ProtocolType::Tcp) {
        net = m_net_->GetNet()->GetNetObject(s.sock);
    }else if (s.protocol_type == ProtocolType::WS) {
        net = m_ws_->GetNet()->GetNetObject(s.sock);
    }
    
    if (!net) {
        dout << "No this sock: " << s.sock << std::endl;
        return;
    }

    if (data.code() == 0) {
        // remove old socket
        auto iter2 = clients_.find(s.account_id);
        if (iter2 != clients_.end()) {
            // kick off old connection;
            rpc::AckKickOff k;
            k.set_time(SquickGetTimeMS());
            //m_net_->SendMsgPB(rpc::ProxyRPC::ACK_KICK_OFF, k, iter2->second.sock);
            return;
        }
        
        net->SetAccountID(s.account_id);
        net->SetConnectKeyState(1);
        rpc::AckConnectProxy ack;
        ack.set_code(0);
        if (s.protocol_type == ProtocolType::Tcp) {
            m_net_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_CONNECT_PROXY, ack.SerializeAsString(), s.sock);
        }
        else if (s.protocol_type == ProtocolType::WS) {
            auto send = ack.SerializeAsString();
            m_ws_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_CONNECT_PROXY, send.data(), send.size(), s.sock);;
        }
        
        auto &client = clients_[s.account_id];
        client.sock = s.sock;
        client.last_ping = SquickGetTimeMSEx();
        client.account_id = s.account_id;
        client.account = data.account();
        client.world_id = data.world_id();
        client.protocol_type = s.protocol_type;
        client.ip = s.ip;

        // 增加schecdule
        m_schedule_->AddSchedule(s.account_id, "HeatbeatCheck", this, &LogicModule::OnHeatbeatCheck, 10.0f, 99999); // 每10秒check一次
        TryEnter(s.account_id);
    } else {
        // if verify failed then close this connect
        //m_net_->GetNet()->CloseNetObject(s.sock);
    }
}

} // namespace proxy::logic