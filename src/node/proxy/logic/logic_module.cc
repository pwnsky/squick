
#include "logic_module.h"
#include <squick/core/base.h>

namespace proxy::logic {

bool LogicModule::Start() {
    last_update_work_load_info_time_ = SquickGetTimeS();
    return true;
}

bool LogicModule::Destroy() { return true; }

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

    m_ws_->AddReceiveCallBack(rpc::ProxyRPC::REQ_CONNECT_PROXY, this, &LogicModule::OnReqConnectWithWS);
    m_ws_->AddReceiveCallBack(this, &LogicModule::OnOtherMessage);
    m_ws_->AddReceiveCallBack(rpc::PlayerRPC::REQ_PLAYER_ENTER, this, &LogicModule::OnReqPlayerEnter);
    m_ws_->AddReceiveCallBack(rpc::PlayerRPC::REQ_PLAYER_LEAVE, this, &LogicModule::OnReqPlayerLeave);

    // Master
    m_net_client_->AddReceiveCallBack(ServerType::ST_MASTER, this, &LogicModule::OnNAckMinWorkloadNodeInfo);

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
    pbreq.add_type_list(ST_PLAYER);
    pbreq.add_type_list(ST_WORLD);
    m_net_client_->SendPBByID(DEFAULT_MASTER_ID, rpc::NMasterRPC::NREQ_MIN_WORKLOAD_NODE_INFO, pbreq);
}

void LogicModule::OnNAckMinWorkloadNodeInfo(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    uint64_t uid;
    rpc::NAckMinWorkloadNodeInfo ack;
    if (!INetModule::ReceivePB(msg_id, msg, len, ack, uid)) {
        return;
    }

    for (auto &info : ack.list()) {
        min_workload_nodes_[info.type()] = info.id();
    }
}

void LogicModule::OnReqPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {

    auto pInfo = GetPlayerConnInfo(sock);
    if (pInfo == nullptr) {
        LOG_WARN("No this player in socket: %v ", sock);
        return;
    }

    int player_node = GetLoadBanlanceNode(ST_PLAYER);
    if (player_node <= 0 || pInfo->player_node > 0) {
        LOG_WARN("No player node find: %v ", player_node);
        return;
    }

    rpc::ReqPlayerEnter req;
    req.set_account_id(pInfo->account_id);
    req.set_account(pInfo->account);
    req.set_ip(pInfo->ip);
    req.set_protocol((int)pInfo->protocol_type);
    req.set_proxy_node(pm_->GetAppID());
    req.set_login_node(pInfo->login_node);
    req.set_area(pm_->GetArea());
    req.set_proxy_sock(sock);
    m_net_client_->SendPBByID(player_node, rpc::PlayerRPC::REQ_PLAYER_ENTER, req);
    pInfo->player_node = player_node;

    LOG_INFO("ReqPlayerEnter get player node:  ", player_node);
    return;
}

void LogicModule::OnAckPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {

    uint64_t uid = 0;

    rpc::AckPlayerEnter ack;
    if (!m_net_->ReceivePB(msg_id, msg, len, ack, uid)) {
        return;
    }

    // start heatbeat
    // m_schedule_->AddSchedule(s.account_id, "HeatbeatCheck", this, &LogicModule::OnHeatbeatCheck, 10.0f, 99999); // 每10秒check一次

    uid = ack.data().uid();
    socket_t player_sock = ack.proxy_sock();

    auto pInfo = GetPlayerConnInfoByUID(uid);
    if (pInfo != nullptr) {
        LOG_ERROR("Uid: %v, %v", uid, " has enter the game, do not try again");
        return;
    }

    pInfo = GetPlayerConnInfo(player_sock);

    if (pInfo == nullptr) {
        LOG_ERROR("Uid: %v no this socket: %v", uid, player_sock);
        return;
    }

    // 在这里创建玩家表
    players_socks_[uid] = player_sock;
    pInfo->uid = uid;
    pInfo->status = PlayerOnline;

    LOG_INFO("Uid: %v has entered the game, sock<%v> account_id<%v> account<%v> protocol_type<%v>", uid, player_sock, pInfo->account_id.c_str(),
             pInfo->account.c_str(), (int)pInfo->protocol_type);

    if (pInfo->protocol_type == ProtocolType::Tcp) {
        m_net_->SendMsg(rpc::PlayerRPC::ACK_PLAYER_ENTER, ack.SerializeAsString(), player_sock);
    } else if (pInfo->protocol_type == ProtocolType::WS) {
        m_ws_->SendPBMsg(rpc::PlayerRPC::ACK_PLAYER_ENTER, ack, player_sock);
        ;
    }
}

int LogicModule::GetLoadBanlanceNode(ServerType type) {
    auto iter = min_workload_nodes_.find(type);
    if (iter == min_workload_nodes_.end()) {
        return 0;
    }
    return iter->second;
}

void LogicModule::OnReqPlayerLeave(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void LogicModule::OnClientDisconnected(const socket_t sock) {
    LOG_INFO("Client disconnected, sock<%v>", sock);
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
        return;
    }

    // 判断是否有新的连接
    // 移除schedule
    // m_schedule_->RemoveSchedule(iter->second.account_id);
    if (pInfo->player_node > 0) {
        // when a net-object bind a account then tell that game-server
        rpc::NNtfPlayerOffline ntf;
        m_net_client_->SendPBByID(pInfo->player_node, rpc::NPlayerRPC::NNTF_PLAYER_OFFLINE, ntf);
    }

    LOG_INFO("The client is disconnected, account_id<%v>", pInfo->account_id);
    RemovePlayerConnInfo(sock);
}

// forward to client
void LogicModule::OnRecivedPlayerNodeMsg(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    rpc::MsgBase msg_pak;
    if (!msg_pak.ParseFromArray(msg, len)) {
        LOG_ERROR("Parse player msg failed from msg_id: <%v> ", msg_id);
        return;
    }
    SendToPlayer(msg_pak.uid(), msg_id, msg_pak.msg_data());
    return;
}

bool LogicModule::SendToPlayer(uint64_t uid, const int msg_id, const string &data) {

    bool ret = false;
    try {
        auto pInfo = GetPlayerConnInfo(uid);
        if (pInfo == nullptr) {
            throw;
        }
        if (pInfo->protocol_type == ProtocolType::Tcp) {
            m_net_->SendMsg(msg_id, data, pInfo->sock);
        } else if (pInfo->protocol_type == ProtocolType::WS) {
            m_ws_->SendMsg(msg_id, data.data(), data.size(), pInfo->sock);
        }
        ret = true;
    } catch (...) {
        LOG_ERROR("Send to player failed, uid<%v>", uid);
    }
    return ret;
}

void LogicModule::OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    auto pInfo = GetPlayerConnInfo(sock);

    if (pInfo == nullptr) {
        LOG_ERROR("OnOtherMessage get player connect info is null, sock<%v> msg_id<%v>", sock, msg_id);
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
        return;
    }

    std::string msgData(msg, len);
    pInfo->last_ping = SquickGetTimeMS();
    if (pInfo->protocol_type == ProtocolType::Tcp) {
        m_net_->SendMsg(rpc::ProxyRPC::ACK_HEARTBEAT, msgData, sock);
    } else if (pInfo->protocol_type == ProtocolType::WS) {
        m_ws_->SendMsg(rpc::ProxyRPC::ACK_HEARTBEAT, msgData.data(), msgData.size(), sock);
    }
}

void LogicModule::OnReqTestProxy(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    INT64 now_time = SquickGetTimeMSEx();
    static INT64 last_time = 0;
    static INT64 request_time = 0;
    static INT64 last_request_times = 0;
    request_time++;

    if (now_time - last_time > 1000000) {
        rpc::Test req;
        uint64_t uid;
        INetModule::ReceivePB(msg_id, string(msg, len), req, uid);
        LOG_INFO("Proxy Test: handle quests: <%v> times/second req network time: <%v> ms", request_time - last_request_times,
                 (now_time - req.req_time()) / 1000.0f);
        last_time = now_time;
        last_request_times = request_time;
    }

    m_net_->SendMsg(rpc::TestRPC::ACK_TEST_PROXY, string(msg, len), sock);
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

PlayerConnInfo *LogicModule::GetPlayerConnInfoByUID(const uint64_t uid) {
    auto iter = players_socks_.find(uid);
    if (iter == players_socks_.end()) {
        return nullptr;
    }
    socket_t sock = iter->second;
    return GetPlayerConnInfo(sock);
}

PlayerConnInfo *LogicModule::GetPlayerConnInfo(const socket_t player_sock) {
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
    uint64_t uid = iter->second.uid;
    players_.erase(iter);

    auto iter2 = players_socks_.find(uid);
    if (iter2 != players_socks_.end()) {
        players_socks_.erase(iter2);
    }

    return true;
}

Coroutine<bool> LogicModule::OnReqConnectWithTcp(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    OnReqConnect(ProtocolType::Tcp, sock, msg_id, msg, len);
    co_return;
}

void LogicModule::OnReqConnectWithWS(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    OnReqConnect(ProtocolType::WS, sock, msg_id, msg, len);
}

void LogicModule::OnReqConnect(ProtocolType type, const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    NetObject *pNetObject = nullptr;
    if (type == ProtocolType::Tcp) {
        pNetObject = m_net_->GetNet()->GetNetObject(sock);
    } else if (type == ProtocolType::WS) {
        pNetObject = m_ws_->GetNet()->GetNetObject(sock);
    }

    if (!pNetObject) {
        return;
    }

    rpc::ReqConnectProxy req;
    if (!req.ParseFromArray(msg, len)) {
        return;
    }
    // check login_node
    int login_node = req.login_node();

    // Verify the key
    Session s;
    s.account_id = req.account_id();
    s.key = req.key();
    s.time = SquickGetTimeMS();
    s.sock = sock;
    s.ip = pNetObject->GetIP();
    s.protocol_type = type;
    s.login_node = login_node;
    sessions_[sock] = s;

    rpc::NReqConnectProxyVerify nreq;
    nreq.set_session(sock);
    nreq.set_key(req.key());
    nreq.set_account_id(req.account_id());

    LOG_INFO("OnReqConnect: account_id<%v> key<%v> session<%v> login_node<%v> ip<%v> protocol_type<%v>", req.account_id(), req.key(), sock, login_node, s.ip,
             (int)type);
    // signatrue check
    //
    // send rand id
    // Modify ...
    m_net_client_->SendPBByID(login_node, rpc::NLoginRPC::NREQ_PROXY_CONNECT_VERIFY, nreq);
}

void LogicModule::OnNAckConnectVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    uint64_t uid;
    rpc::NAckConnectProxyVerify data;
    if (!m_net_->ReceivePB(msg_id, msg, len, data, uid)) {
        return;
    }

    // find session
    auto session = data.session();
    auto iter = sessions_.find(session);
    if (iter == sessions_.end()) {
        // session not exsist
        LOG_ERROR(" No this session<%v>", session);
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
            m_net_->SendMsg(rpc::ProxyRPC::ACK_CONNECT_PROXY, ack.SerializeAsString(), s.sock);
        } else if (s.protocol_type == ProtocolType::WS) {
            m_ws_->SendPBMsg(rpc::ProxyRPC::ACK_CONNECT_PROXY, ack, s.sock);
            ;
        }

        auto &client = players_[s.sock];
        client.sock = s.sock;
        client.last_ping = SquickGetTimeMSEx();
        client.account_id = s.account_id;
        client.account = data.account();
        client.login_node = s.login_node;
        // client.world_id = data.world_id();
        client.protocol_type = s.protocol_type;
        client.ip = s.ip;
        client.status = PlayerNotEneter;

        LOG_INFO("Verify succ! account_id<%v>", client.account_id);
    } else {
        // if verify failed then close this connect
        // m_net_->GetNet()->CloseNetObject(s.sock);
    }
}

} // namespace proxy::logic
