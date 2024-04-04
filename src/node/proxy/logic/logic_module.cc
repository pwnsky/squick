
#include "logic_module.h"

namespace proxy::logic {

bool LogicModule::Start() { return true; }

bool LogicModule::Destory() { return true; }

bool LogicModule::Update() {

    time_t now_time = SquickGetTimeS();
    if (now_time - last_update_work_load_info_time_ >= 10) {
        NReqMinWorkloadNodeInfo();
        last_update_work_load_info_time_ = now_time;
    }

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
    m_net_->AddReceiveCallBack(rpc::PlayerRPC::REQ_PLAYER_ENTER, this, &LogicModule::OnReqPlayerEnter);
    m_net_->AddReceiveCallBack(rpc::PlayerRPC::REQ_PLAYER_LEAVE, this, &LogicModule::OnReqPlayerLeave);
    m_net_->AddReceiveCallBack(rpc::TestRPC::REQ_TEST_PROXY, this, &LogicModule::OnReqTestProxy);
    m_net_client_->AddReceiveCallBack(ServerType::ST_MASTER, this, &LogicModule::OnNAckMinWorkloadNodeInfo);
    m_ws_->AddReceiveCallBack(rpc::ProxyRPC::REQ_CONNECT_PROXY, this, &LogicModule::OnReqConnectWithWS);
    m_ws_->AddReceiveCallBack(this, &LogicModule::OnOtherMessage);

    // Lobby
    m_net_client_->AddReceiveCallBack(ServerType::ST_PLAYER, this, &LogicModule::OnRecivedPlayerNodeMsg);
    m_net_client_->AddReceiveCallBack(ServerType::ST_LOGIN, rpc::NLoginRPC::NACK_PROXY_CONNECT_VERIFY, this, &LogicModule::OnNAckConnectVerify);
    m_net_client_->AddReceiveCallBack(ServerType::ST_PLAYER, rpc::PlayerRPC::ACK_PLAYER_ENTER, this, &LogicModule::OnAckPlayerEnter);
    return true;
}

// request per 5 sec
void LogicModule::NReqMinWorkloadNodeInfo() {
    // find min work load proxy
    rpc::NReqMinWorkloadNodeInfo pbreq;
    pbreq.add_type_list(ST_PROXY);
    m_net_client_->SendPBByID(DEFAULT_NODE_MASTER_ID, rpc::NMasterRPC::NREQ_MIN_WORKLOAD_NODE_INFO, pbreq);

}

void LogicModule::OnNAckMinWorkloadNodeInfo(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    string guid;
    rpc::NAckMinWorkloadNodeInfo ack;
    if (!INetModule::ReceivePB(msg_id, msg, len, ack, guid)) {
        return;
    }

    for (auto &info : ack.list()) {
        min_workload_nodes_[info.type()] = info.id();
    }

    dout << "OnNAckMinWorkloadNodeInfo: updated !\n";
}

void LogicModule::OnReqPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "OnReqPlayerEnter\n";
    auto pInfo = GetPlayerConnInfo(sock);
    if (pInfo == nullptr) {
        return;
    }

    int player_node = GetLoadBanlanceNode(ST_PLAYER);
    if (player_node == -1 || pInfo->player_node > 0) {
        return;
    }

    m_net_client_->SendByID(player_node, rpc::PlayerRPC::REQ_PLAYER_ENTER, std::string(msg, len));
    pInfo->player_node = player_node;
    return;
}

void LogicModule::OnAckPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {

    rpc::AckPlayerEnter ack;
    string guid;
    if (!ack.ParseFromString(std::string(msg, len))) {
        return;
    }

    dout << "OnAckPlayerEnter\n";
    // start heatbeat
    //m_schedule_->AddSchedule(s.account_id, "HeatbeatCheck", this, &LogicModule::OnHeatbeatCheck, 10.0f, 99999); // 每10秒check一次
    auto pInfo = GetPlayerConnInfo(ack.uid());

    if (pInfo != nullptr) {
        return ;
    }

    // 在这里创建玩家表
}

int GetLoadBanlanceNode(ServerType type) {
    return 0;
}


void LogicModule::OnReqPlayerLeave(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {

}

void LogicModule::OnClientDisconnected(const socket_t sock) {
    dout << sock << " client disconnected\n";
    // remove session
    auto iter = sessions_.find(sock);
    if (iter != sessions_.end()) {
        sessions_.erase(iter);
    }
    
    NetObject *net_obj = m_net_->GetNet()->GetNetObject(sock);
    if (net_obj) {
        m_net_->GetNet()->CloseNetObject(sock);
    }

    auto pInfo = GetPlayerConnInfo(sock);
    if (pInfo == nullptr) {
        return ;
    }

    // 判断是否有新的连接
    // 移除schedule
    // m_schedule_->RemoveSchedule(iter->second.account_id);
    if (pInfo->player_node > 0) {
        // when a net-object bind a account then tell that game-server
        rpc::NNtfPlayerOffline ntf;
        m_net_client_->SendPBByID(pInfo->player_node, rpc::NPlayerRPC::NNTF_PLAYER_OFFLINE, ntf);
    }

    dout << "client disconnected " << pInfo->account_id << std::endl;
    //return;

    RemovePlayerConnInfo(sock);
}

// forward to client
void LogicModule::OnRecivedPlayerNodeMsg(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << " ForwardToClient: " << std::endl;
    rpc::MsgBase msg_pak;
    if (!msg_pak.ParseFromArray(msg, len)) {
        char szData[MAX_PATH] = {0};
        sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
        return;
    }

    // broadcast many palyers
    for (int i = 0; i < msg_pak.broadcast_size(); ++i) {
        //SendToPlayer(xMsg.broadcast(i), msg_id, xMsg.msg_data());
    }

    // send to one player
    if (msg_pak.broadcast_size() <= 0) {
        SendToPlayer(msg_pak.uid(), msg_id, msg_pak.msg_data());
    }
    return;
}

bool LogicModule::SendToPlayer(uint64_t uid, const int msg_id, const string &data) {
    
    bool ret = false;
    try {
        auto pInfo = GetPlayerConnInfo(uid);
        if (pInfo == nullptr) {
            throw;
        }

        dout << " SendToPlayer: " << uid << " msg_id: " << msg_id << " socks: " << pInfo->sock << std::endl;
        if (pInfo->protocol_type == ProtocolType::Tcp) {
            m_net_->SendMsgWithOutHead(msg_id, data, pInfo->sock);
        } else if (pInfo->protocol_type == ProtocolType::WS) {
            m_ws_->SendMsgWithOutHead(msg_id, data.data(), data.size(), pInfo->sock);
        }
        ret = true;
    }
    catch (...) {
        ostringstream info;
        info << "Send to player failed, uid: " << uid;
        m_log_->LogError(info);
    }
    return ret;
}


void LogicModule::OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    auto pInfo = GetPlayerConnInfo(sock);

    if (pInfo == nullptr) {
        dout << "sock: " << sock << " msg_id" << msg_id << " cannot send msg\n";
        return;
    }
    rpc::MsgBase xMsg;
    xMsg.set_uid(pInfo->uid);
    *xMsg.mutable_msg_data() = string(msg, len);
    std::string pak;
    if (!xMsg.SerializeToString(&pak)) {
        return;
    }
    // 根据ID 来转发至不同服务器
    if (msg_id >= 12000 && msg_id < 30000) {
        // 转发到Player node
        int node_player_id = pInfo->player_node;
        if (node_player_id <= 0) {
                return;
        }
        m_net_client_->SendByID(node_player_id, msg_id, pak);
    } else if (msg_id >= 10000 && msg_id < 32000) {

    } else if (msg_id >= 30000 && msg_id < 32000) {

    } else if (msg_id >= 32000 && msg_id < 35000) {

    } else {
        // 不支持转发
    }
}

void LogicModule::OnHeartbeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    auto pInfo = GetPlayerConnInfo(sock);
    if (pInfo == nullptr) {
        return ;
    }

    std::string msgData(msg, len);
    pInfo->last_ping = SquickGetTimeMS();
    if (pInfo->protocol_type == ProtocolType::Tcp) {
        m_net_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_HEARTBEAT, msgData, sock);
    } else if(pInfo->protocol_type == ProtocolType::WS){
        m_ws_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_HEARTBEAT, msgData.data(), msgData.size(), sock);
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
    /*
    auto iter = players_.find(self.ToString());
    if (iter == players_.end()) {
        dout << "No this player to heatbeat check\n";
        return 1;
    }
    // dout << "heatbeatcheck : " << self.ToString() << "  " << iter->second.last_ping << "  now " << SquickGetTimeMS() << std::endl;
    time_t now = SquickGetTimeMS();
    if (now - iter->second.last_ping > 30000) { // 大于30秒即断线
        OnClientDisconnected(iter->second.sock);
    }
    return 0;*/
    return 0;
}

PlayerConnInfo* LogicModule::GetPlayerConnInfo(const uint64_t uid) {
    auto iter = players_socks_.find(uid);
    if (iter == players_socks_.end()) {
        return nullptr;
    }
    socket_t sock = iter->second;
    return GetPlayerConnInfo(sock);
}

PlayerConnInfo* LogicModule::GetPlayerConnInfo(const socket_t player_sock) {
    auto iter = players_.find(player_sock);
    if (iter == players_.end()) {
        return nullptr;
    }
    return &iter->second;
}

bool LogicModule::RemovePlayerConnInfo(const socket_t player_sock) {
    auto iter = players_.find(player_sock);
    if (iter == players_.end()) {
        return false;
    }
    players_.erase(iter);
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

    rpc::NReqConnectProxyVerify nreq;
    nreq.set_session(sock);
    nreq.set_key(req.key());
    nreq.set_account_id(req.account_id());

    int login_node = req.login_node();
    // check login_node

    dout << "login_node: " << login_node << endl;
    // send rand id
    // Modify ...
    m_net_client_->SendPBByID(login_node, rpc::NLoginRPC::NREQ_PROXY_CONNECT_VERIFY, nreq);
}

void LogicModule::OnNAckConnectVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    string tmp;
    rpc::NAckConnectProxyVerify data;
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

    if (data.code() == 0) {
        // remove old socket
        /*
        auto iter2 = clients_.find(s.account_id);
        if (iter2 != clients_.end()) {
            // kick off old connection;
            rpc::AckKickOff k;
            k.set_time(SquickGetTimeMS());
            //m_net_->SendMsgPB(rpc::ProxyRPC::ACK_KICK_OFF, k, iter2->second.sock);
            return;
        }*/
        
        rpc::AckConnectProxy ack;
        ack.set_code(0);
        if (s.protocol_type == ProtocolType::Tcp) {
            m_net_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_CONNECT_PROXY, ack.SerializeAsString(), s.sock);
        }
        else if (s.protocol_type == ProtocolType::WS) {
            auto send = ack.SerializeAsString();
            m_ws_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_CONNECT_PROXY, send.data(), send.size(), s.sock);;
        }
        
        auto &client = players_[s.sock];
        client.sock = s.sock;
        client.last_ping = SquickGetTimeMSEx();
        client.account_id = s.account_id;
        client.account = data.account();
        //client.world_id = data.world_id();
        client.protocol_type = s.protocol_type;
        client.ip = s.ip;

        dout << "Verify succ! " << client.account_id << endl;
    } else {
        // if verify failed then close this connect
        //m_net_->GetNet()->CloseNetObject(s.sock);
    }
}

} // namespace proxy::logic
