
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
    m_net_->AddReceiveCallBack(rpc::LobbyBaseRPC::REQ_ENTER, this, &LogicModule::OnReqEnterGameServer);
    m_net_->AddReceiveCallBack(rpc::TestRPC::REQ_TEST_PROXY, this, &LogicModule::OnReqTestProxy);

    return true;
}

void LogicModule::OnClientDisconnect(const socket_t sock) {
    // remove session
    auto iter = sessions_.find(sock);
    if (iter != sessions_.end()) {
        sessions_.erase(iter);
    }

    dout << "client disconnected\n";
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (pNetObject) {

        if (pNetObject->GetClientID().IsNull()) {
            return;
        }

        auto iter = clients_.find(pNetObject->GetClientID().ToString());
        if (iter != clients_.end()) {
            // 判断是否有新的连接
            if (iter->second.sock == sock) {
                // 移除schedule
                m_schedule_->RemoveSchedule(iter->second.guid);
                clients_.erase(iter);

                int nGameID = pNetObject->GetGameID();
                if (nGameID > 0) {

                    // when a net-object bind a account then tell that game-server
                    if (!pNetObject->GetUserID().IsNull()) {
                        rpc::ReqLeave xData;
                        rpc::MsgBase xMsg;

                        // real user id
                        *xMsg.mutable_player_id() = INetModule::StructToProtobuf(pNetObject->GetUserID());

                        if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
                            return;
                        }

                        std::string msg;
                        if (!xMsg.SerializeToString(&msg)) {
                            return;
                        }

                        m_net_client_->SendByServerIDWithOutHead(nGameID, rpc::LobbyBaseRPC::REQ_LEAVE, msg);

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
    for (int i = 0; i < xMsg.player_client_list_size(); ++i) {
        Guid target = INetModule::ProtobufToStruct(xMsg.player_client_list(i));
        auto iter = clients_.find(target.ToString());
        if (iter != clients_.end()) {
            if (xMsg.has_hash_ident()) {
                NetObject *pNetObject = m_net_->GetNet()->GetNetObject(iter->second.sock);
                if (pNetObject) {
                    pNetObject->SetHashIdentID(INetModule::ProtobufToStruct(xMsg.hash_ident()));
                }
            }
            m_net_->SendMsgWithOutHead(msg_id, std::string(msg, len), iter->second.sock);
        }
    }

    // send message to one player
    if (xMsg.player_client_list_size() <= 0) {
        Guid target = INetModule::ProtobufToStruct(xMsg.player_id());
        auto iter = clients_.find(target.ToString());
        if (iter != clients_.end()) {
            if (xMsg.has_hash_ident()) {
                NetObject *pNetObject = m_net_->GetNet()->GetNetObject(iter->second.sock);
                if (pNetObject) {
                    pNetObject->SetHashIdentID(INetModule::ProtobufToStruct(xMsg.hash_ident()));
                }
            }

            m_net_->SendMsgWithOutHead(msg_id, std::string(msg, len), iter->second.sock);
        }
    }
    return true;
}

void LogicModule::OnClientConnected(const socket_t sock) { std::cout << "Client Connected.... \n"; }

int LogicModule::EnterGameSuccessEvent(const Guid account_guid, const Guid object_guid) {
    auto iter = clients_.find(account_guid.ToString());
    if (iter != clients_.end()) {
        NetObject *pNetObeject = m_net_->GetNet()->GetNetObject(iter->second.sock);
        if (pNetObeject) {
            pNetObeject->SetUserID(object_guid);
        }
    }
    return 0;
}

void LogicModule::OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << " forward...\n";
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (!pNetObject || pNetObject->GetConnectKeyState() <= 0 || pNetObject->GetGameID() <= 0) {
        // state error
        return;
    }

    Guid guid = pNetObject->GetUserID().ToString();
    // check
    auto iter = clients_.find(guid.ToString());
    if (iter == clients_.end()) {
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
    *xMsg.mutable_player_id() = INetModule::StructToProtobuf(pNetObject->GetUserID());
    // dout << "proxy get real user guid: " << pNetObject->GetUserID().ToString() << std::endl;

    std::string msgData;
    if (!xMsg.SerializeToString(&msgData)) {
        return;
    }
    // 游戏服
    if (xMsg.has_hash_ident()) {
        // special for distributed
        if (!pNetObject->GetHashIdentID().IsNull()) {
            m_net_client_->SendBySuitWithOutHead(ServerType::ST_GAME, pNetObject->GetHashIdentID().ToString(), msg_id, msgData);
        } else {
            Guid xHashIdent = INetModule::ProtobufToStruct(xMsg.hash_ident());
            m_net_client_->SendBySuitWithOutHead(ServerType::ST_GAME, xHashIdent.ToString(), msg_id, msgData);
        }
    } // 大区服
    else {
        if (msg_id >= 50000) {
            m_net_client_->SendBySuitWithOutHead(ServerType::ST_WORLD, pNetObject->GetUserID().ToString(), msg_id, msgData);
        } else {
            m_net_client_->SendByServerIDWithOutHead(pNetObject->GetGameID(), msg_id, msgData);
        }
    }
    // 微服
}

void LogicModule::OnHeartbeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << " OnHeatbeat\n";
    std::string msgData(msg, len);
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (pNetObject) {
        auto iter = clients_.find(pNetObject->GetClientID().ToString());
        if (iter != clients_.end()) {
            iter->second.last_ping = SquickGetTimeMS();
            dout << "heatbeat: " << iter->second.last_ping << std::endl;
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
    if (now - iter->second.last_ping > 10000) { // 大于8秒即断线
        OnClientDisconnect(iter->second.sock);
    }
    return 0;
}

// 选择服务器
bool LogicModule::SelectGameServer(int sock) {
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (!pNetObject) {
        return false;
    }

    // 查找最小负载的服务器
    // actually, if you want the game server working with a good performance then we need to find the game server with lowest workload
    int work_load = 999999;
    int id = 0;
    MapEx<int, ConnectData> &xServerList = m_net_client_->GetServerList();
    ConnectData *c = xServerList.FirstNude();
    while (c) {
        if (ConnectDataState::NORMAL == c->state && ServerType::ST_GAME == c->type) {
            if (c->work_load < work_load) {
                work_load = c->work_load;
                id = c->id;
            }
        }
        c = xServerList.NextNude();
    }

    if (id > 0) {
        pNetObject->SetGameID(id);
        return true;
    }

    // 如果服务器比较繁忙，给客户端响应繁忙
    return false;
}

// 请求进入游戏
void LogicModule::OnReqEnterGameServer(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "请求进入游戏\n";

    SelectGameServer(sock);

    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (!pNetObject) {
        return;
    }

    Guid nPlayerID; // no value
    rpc::ReqEnter xData;
    if (!m_net_->ReceivePB(msg_id, msg, len, xData, nPlayerID)) {
        return;
    }

    // check
    Guid guid = pNetObject->GetUserID().ToString();
    auto iter = clients_.find(guid.ToString());
    if (iter == clients_.end()) {
        dout << "This player not exsisted\n";
        return;
    }

    // Set real info
    *xData.mutable_account() = iter->second.account;
    *xData.mutable_guid() = INetModule::StructToProtobuf(iter->second.guid);

    std::shared_ptr<ConnectData> pServerData = m_net_client_->GetServerNetInfo(pNetObject->GetGameID());
    if (pServerData && ConnectDataState::NORMAL == pServerData->state) {
        if (pNetObject->GetConnectKeyState() > 0) {
            rpc::MsgBase xMsg;
            if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
                return;
            }
            // clientid
            xMsg.mutable_player_id()->CopyFrom(INetModule::StructToProtobuf(pNetObject->GetClientID()));
            std::string msg;
            if (!xMsg.SerializeToString(&msg)) {
                return;
            }

            m_net_client_->SendByServerIDWithOutHead(pNetObject->GetGameID(), rpc::LobbyBaseRPC::REQ_ENTER, msg);
        }
    }
}

void LogicModule::OnReqConnect(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (!pNetObject) {
        return;
    }

    Guid nPlayerID;
    rpc::ReqConnectProxy req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, nPlayerID)) {
        return;
    }

    // 验证Token
    // bool bRet = m_pProxyToWorldModule->VerifyConnectData(xMsg.account(), xMsg.security_code());
    Guid guid = INetModule::ProtobufToStruct(req.guid());
    Session s;
    s.guid = guid;
    s.key = req.key();
    s.time = SquickGetTimeMS();
    s.sock = sock;
    sessions_[sock] = s;
    m_node_->OnReqProxyConnectVerify(sock, guid.ToString(), req.key());
}

void LogicModule::OnAckConnectVerify(const int msg_id, const char *msg, const uint32_t len) {
    dout << "验证响应\n";
    Guid tmp;
    rpc::AckConnectProxyVerify data;
    if (!m_net_->ReceivePB(msg_id, msg, len, data, tmp)) {
        dout << "反序列化失败\n";
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

    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(s.sock);
    if (!pNetObject) {
        dout << "No this sock: " << s.sock << std::endl;
        return;
    }

    if (data.code() == 0) {
        // remove old socket
        auto iter2 = clients_.find(s.guid.ToString());
        if (iter2 != clients_.end()) {
            // kick off old connection;
            rpc::AckKickOff k;
            k.set_time(SquickGetTimeMS());
            m_net_->SendMsgPB(rpc::ProxyRPC::ACK_KICK_OFF, k, iter2->second.sock);
            // m_net_->GetNet()->CloseNetObject(iter2->second.sock);
        }

        // bind account guid with socket
        pNetObject->SetClientID(s.guid);

        // NetObject* pNetObject = m_net_->GetNet()->GetNetObject(sock);
        pNetObject->SetConnectKeyState(1);
        pNetObject->SetSecurityKey(s.key);
        // this net-object bind a user's account
        pNetObject->SetAccount(s.guid.ToString());
        pNetObject->SetUserID(s.guid);
        rpc::AckConnectProxy ack;
        dout << s.guid.ToString() << " 连接成功!\n";
        ack.set_code(0);
        m_net_->SendMsgPB(rpc::ProxyRPC::ACK_CONNECT_PROXY, ack, s.sock);
        string sguid = s.guid.ToString();
        auto &client = clients_[sguid];
        client.sock = s.sock;
        client.last_ping = SquickGetTimeMSEx();
        client.guid = s.guid;
        client.account = data.account();
        client.world_id = data.world_id();

        // 增加schecdule
        m_schedule_->AddSchedule(s.guid, "HeatbeatCheck", this, &LogicModule::OnHeatbeatCheck, 10.0f, 99999); // 每10秒check一次

    } else {
        dout << "验证失败 code: " << data.code() << "\n";
        // if verify failed then close this connect
        m_net_->GetNet()->CloseNetObject(s.sock);
    }
}

} // namespace proxy::logic