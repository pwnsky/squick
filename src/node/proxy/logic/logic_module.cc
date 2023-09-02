
#include "logic_module.h"
#include "plugin.h"

namespace proxy::logic {

bool LogicModule::Start() { return true; }

bool LogicModule::Destory() { return true; }

bool LogicModule::Update() { return true; }

bool LogicModule::AfterStart() {
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_schedule_ = pm_->FindModule<IScheduleModule>();
    m_node_ = pm_->FindModule<node::INodeModule>();

    m_net_->AddReceiveCallBack(this, &LogicModule::OnOtherMessage);
    m_net_->AddReceiveCallBack(rpc::ProxyRPC::REQ_HEARTBEAT, this, &LogicModule::OnHeartbeat);
    m_net_->AddReceiveCallBack(rpc::ProxyRPC::REQ_CONNECT_PROXY, this, &LogicModule::OnReqConnect);
    m_net_->AddReceiveCallBack(rpc::TestRPC::REQ_TEST_PROXY, this, &LogicModule::OnReqTestProxy);
    return true;
}

void LogicModule::OnClientConnected(const socket_t sock) {
    // remove session
    auto iter = sessions_.find(sock);
    if (iter != sessions_.end()) {
        sessions_.erase(iter);
    }

    dout << "client disconnected\n";
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (pNetObject) {
        if (pNetObject->GetAccountID().empty()) {
            return;
        }
        string player_id = pNetObject->GetPlayerID();
        string account_id = pNetObject->GetAccountID();
        auto iter = clients_.find(account_id);
        if (iter != clients_.end()) {
            // 判断是否有新的连接
            if (iter->second.sock == sock) {
                // 移除schedule
                m_schedule_->RemoveSchedule(iter->second.guid);
                clients_.erase(iter);

                int nGameID = pNetObject->GetGameID();
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

                        m_net_client_->SendByServerID(nGameID, rpc::PlayerEventRPC::PLAYER_LEAVE_EVENT, msg);

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
        m_net_->SendMsgWithOutHead(msg_id, data, client.sock);
        ret = true;
    }
    catch (...) {
        ostringstream info;
        info << "Send to player failed, player_id: " << player_id;
        m_log_->LogError(info);
    }
    return ret;
}

int LogicModule::EnterSuccessEvent(const string account_guid, const string object_guid) {
    // 在这里创建玩家表

    auto iter = clients_.find(account_guid);
    if (iter != clients_.end()) {
        NetObject *obj = m_net_->GetNet()->GetNetObject(iter->second.sock);
        if (obj) {
            obj->SetPlayerID(object_guid);
            players_[object_guid] = account_guid;
        }
        m_net_->SendMsgWithOutHead(rpc::PlayerRPC::ACK_PLAYER_ONLINE, "", obj->GetRealFD());
    }
    return 0;
}

void LogicModule::OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (!pNetObject || pNetObject->GetConnectKeyState() <= 0 || pNetObject->GetGameID() <= 0) {
        return;
    }

    string player_id = pNetObject->GetPlayerID();
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

    // real user id
    xMsg.set_guid(player_id);
    std::string msgData;
    if (!xMsg.SerializeToString(&msgData)) {
        return;
    }

    // 根据ID 来转发至不同服务器
    if (msg_id >= 12000 && msg_id < 30000) {
        // 转发到Lobby
        m_net_client_->SendByServerID(ServerType::ST_GAME, msg_id, msgData);
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
        }
    }
    m_net_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_HEARTBEAT, msgData, sock);
}

void LogicModule::OnReqTestProxy(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    INT64 now_time = SquickGetTimeMSEx();
    static INT64 last_time = 0;
    static INT64 request_time = 0;
    static INT64 last_request_times = 0;
    request_time++;

    
    if (now_time - last_time > 1000000) {
        rpc::Test req;
        Guid guid;
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
    if (now - iter->second.last_ping > 10000) { // 大于10秒即断线
        OnClientConnected(iter->second.sock);
    }
    return 0;
}

// 进入游戏
bool LogicModule::TryEnter(string guid) {
    
    auto client = clients_.find(guid);
    if (client == clients_.end()) {
        return false;
    }
    int lobby_id = m_node_->GetLoadBanlanceNode(ServerType::ST_LOBBY);
    if (lobby_id == -1) {
        return false;
    }
    rpc::PlayerEnterEvent event;
    *event.mutable_account() = client->second.account;
    *event.mutable_guid() = client->second.guid;
    event.set_proxy_id(pm_->GetAppID());
    m_net_client_->SendToServerByPB(lobby_id, rpc::PlayerEventRPC::PLAYER_ENTER_EVENT, event);
    client->second.lobby_id = lobby_id;
    return true;
}

void LogicModule::OnReqConnect(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (!pNetObject) {
        return;
    }

    rpc::ReqConnectProxy req;
    if (!req.ParseFromArray(msg, len)) {
        return;
    }
    // 验证Token
    Session s;
    s.guid = req.guid();
    s.key = req.key();
    s.time = SquickGetTimeMS();
    s.sock = sock;
    sessions_[sock] = s;
    m_node_->OnReqProxyConnectVerify(sock, req.guid(), req.key());
}

void LogicModule::OnAckConnectVerify(const int msg_id, const char *msg, const uint32_t len) {
    Guid tmp;
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

    NetObject *net = m_net_->GetNet()->GetNetObject(s.sock);
    if (!net) {
        dout << "No this sock: " << s.sock << std::endl;
        return;
    }

    if (data.code() == 0) {
        // remove old socket
        auto iter2 = clients_.find(s.guid);
        if (iter2 != clients_.end()) {
            // kick off old connection;
            rpc::AckKickOff k;
            k.set_time(SquickGetTimeMS());
            m_net_->SendMsgPB(rpc::ProxyRPC::ACK_KICK_OFF, k, iter2->second.sock);
            // m_net_->GetNet()->CloseNetObject(iter2->second.sock);
            return;
        }

        // bind account guid with socket
        net->SetAccountID(s.guid);
        net->SetConnectKeyState(1);
        //net->SetPlayerID(s.guid);
        rpc::AckConnectProxy ack;
        ack.set_code(0);
        m_net_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_CONNECT_PROXY, ack.SerializeAsString(), s.sock);
        string sguid = s.guid;
        auto &client = clients_[sguid];
        client.sock = s.sock;
        client.last_ping = SquickGetTimeMSEx();
        client.guid = s.guid;
        client.account = data.account();
        client.world_id = data.world_id();

        // 增加schecdule
        m_schedule_->AddSchedule(s.guid, "HeatbeatCheck", this, &LogicModule::OnHeatbeatCheck, 10.0f, 99999); // 每10秒check一次
        TryEnter(sguid);
    } else {
        // if verify failed then close this connect
        //m_net_->GetNet()->CloseNetObject(s.sock);
    }
}

} // namespace proxy::logic