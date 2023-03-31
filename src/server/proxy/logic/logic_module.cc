
#include "logic_module.h"
#include "plugin.h"

namespace proxy::logic {

bool LogicModule::Start() { return true; }

bool LogicModule::Destory() { return true; }

bool LogicModule::Update() {
    // 检查过期

    return true;
}

bool LogicModule::AfterStart() {
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_log_ = pm_->FindModule<ILogModule>();

    m_net_->AddReceiveCallBack(this, &LogicModule::OnOtherMessage);
    m_net_->AddReceiveCallBack(rpc::ProxyRPC::REQ_HEARTBEAT, this, &LogicModule::OnHeartbeat);
    m_net_->AddReceiveCallBack(rpc::ProxyRPC::REQ_CONNECT_PROXY, this, &LogicModule::OnReqConnect);
    m_net_->AddReceiveCallBack(rpc::GameLobbyRPC::REQ_ENTER, this, &LogicModule::OnReqEnterGameServer);
    
    return true;
}

void LogicModule::OnClientDisconnect(const socket_t sock) {
    dout << "client disconnected\n";
    NetObject* pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (pNetObject) {
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

                m_net_client_->SendByServerIDWithOutHead(nGameID, rpc::GameLobbyRPC::REQ_LEAVE, msg);
            }
        }
        mxClientIdent.RemoveElement(pNetObject->GetClientID());
    }

}

// forward to client
int LogicModule::ForwardToClient(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    rpc::MsgBase xMsg;
    if (!xMsg.ParseFromArray(msg, len)) {
        char szData[MAX_PATH] = { 0 };
        sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
        return false;
    }

    // broadcast many palyers
    for (int i = 0; i < xMsg.player_client_list_size(); ++i) {
        std::shared_ptr<socket_t> pFD = mxClientIdent.GetElement(INetModule::ProtobufToStruct(xMsg.player_client_list(i)));
        if (pFD) {
            if (xMsg.has_hash_ident()) {
                NetObject* pNetObject = m_net_->GetNet()->GetNetObject(*pFD);
                if (pNetObject) {
                    pNetObject->SetHashIdentID(INetModule::ProtobufToStruct(xMsg.hash_ident()));
                }
            }
            m_net_->SendMsgWithOutHead(msg_id, std::string(msg, len), *pFD);
        }
    }

    // send message to one player
    if (xMsg.player_client_list_size() <= 0) {
        Guid xClientIdent = INetModule::ProtobufToStruct(xMsg.player_id());
        std::shared_ptr<socket_t> pFD = mxClientIdent.GetElement(xClientIdent);
        if (pFD) {
            if (xMsg.has_hash_ident()) {
                NetObject* pNetObject = m_net_->GetNet()->GetNetObject(*pFD);
                if (pNetObject) {
                    pNetObject->SetHashIdentID(INetModule::ProtobufToStruct(xMsg.hash_ident()));
                }
            }

            m_net_->SendMsgWithOutHead(msg_id, std::string(msg, len), *pFD);
        }
        else if (xClientIdent.IsNull()) {
            // send this msessage to all clientss
            m_net_->GetNet()->SendMsgToAllClientWithOutHead(msg_id, msg, len);
        }
        // pFD is empty means end of connection, no need to send message to this client any more. And,
        // we should never send a message that specified to a player to all clients here.
        else {
        }
    }
    return true;
}

void LogicModule::OnClientConnected(const socket_t sock) {
    std::cout << "Client Connected.... \n";
}

int LogicModule::EnterGameSuccessEvent(const Guid xClientID, const Guid xPlayerID) {
    std::shared_ptr<socket_t> pFD = mxClientIdent.GetElement(xClientID);
    if (pFD) {
        NetObject* pNetObeject = m_net_->GetNet()->GetNetObject(*pFD);
        if (pNetObeject) {
            pNetObeject->SetUserID(xPlayerID);
        }
    }
    return 0;
}


void LogicModule::OnOtherMessage(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    NetObject* pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (!pNetObject || pNetObject->GetConnectKeyState() <= 0 || pNetObject->GetGameID() <= 0) {
        // state error
        return;
    }

    rpc::MsgBase xMsg;
    if (!xMsg.ParseFromString(std::string(msg, len))) {
        char szData[MAX_PATH] = { 0 };
        sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);

        m_log_->LogError(Guid(0, sock), szData, __FUNCTION__, __LINE__);
        return;
    }

    // real user id
    *xMsg.mutable_player_id() = INetModule::StructToProtobuf(pNetObject->GetUserID());
    dout << "proxy get real user guid: " << pNetObject->GetUserID().ToString() << std::endl;

    std::string msgData;
    if (!xMsg.SerializeToString(&msgData)) {
        return;
    }

    if (xMsg.has_hash_ident()) {
        // special for distributed
        if (!pNetObject->GetHashIdentID().IsNull()) {
            m_net_client_->SendBySuitWithOutHead(ServerType::ST_GAME, pNetObject->GetHashIdentID().ToString(), msg_id, msgData);
        }
        else {
            Guid xHashIdent = INetModule::ProtobufToStruct(xMsg.hash_ident());
            m_net_client_->SendBySuitWithOutHead(ServerType::ST_GAME, xHashIdent.ToString(), msg_id, msgData);
        }
    }
    else {
        if (msg_id >= 50000) {
            m_net_client_->SendBySuitWithOutHead(ServerType::ST_WORLD, pNetObject->GetUserID().ToString(), msg_id, msgData);
        }
        else {
            m_net_client_->SendByServerIDWithOutHead(pNetObject->GetGameID(), msg_id, msgData);
        }
    }
}


void LogicModule::OnHeartbeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    std::string msgData(msg, len);
    m_net_->SendMsgWithOutHead(rpc::ProxyRPC::ACK_HEARTBEAT, msgData, sock);
}


// 选择服务器
bool LogicModule::SelectGameServer(int sock) {
    NetObject* pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (!pNetObject) {
        return false;
    }

    // 查找最小负载的服务器
    // actually, if you want the game server working with a good performance then we need to find the game server with lowest workload
    int nWorkload = 999999;
    int nGameID = 0;
    MapEx<int, ConnectData>& xServerList = m_net_client_->GetServerList();
    ConnectData* pGameData = xServerList.FirstNude();
    while (pGameData) {
        if (ConnectDataState::NORMAL == pGameData->eState && ServerType::ST_GAME == pGameData->eServerType) {
            if (pGameData->nWorkLoad < nWorkload) {
                nWorkload = pGameData->nWorkLoad;
                nGameID = pGameData->nGameID;
            }
        }
        pGameData = xServerList.NextNude();
    }

    if (nGameID > 0) {
        pNetObject->SetGameID(nGameID);
        return true;
    }

    // 如果服务器比较繁忙，给客户端响应繁忙
    return false;
}

// 请求进入游戏
void LogicModule::OnReqEnterGameServer(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    dout << "请求进入游戏\n";

    SelectGameServer(sock);

    NetObject* pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (!pNetObject) {
        return;
    }

    Guid nPlayerID; // no value
    rpc::ReqEnter xData;
    if (!m_net_->ReceivePB(msg_id, msg, len, xData, nPlayerID)) {
        return;
    }

    std::shared_ptr<ConnectData> pServerData = m_net_client_->GetServerNetInfo(pNetObject->GetGameID());
    if (pServerData && ConnectDataState::NORMAL == pServerData->eState) {
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

            m_net_client_->SendByServerIDWithOutHead(pNetObject->GetGameID(), rpc::GameLobbyRPC::REQ_ENTER, msg);
        }
    }
}


void LogicModule::OnReqConnect(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    Guid nPlayerID;
    rpc::ReqConnectProxy req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, nPlayerID))
    {
        return;
    }

    // 验证Token
    //bool bRet = m_pProxyToWorldModule->VerifyConnectData(xMsg.account(), xMsg.security_code());
    Guid guid = INetModule::ProtobufToStruct(req.guid());
    bool bRet = true;
    if (bRet)
    {
        // bind user id with socket
        Guid xClientIdent = guid;
        NetObject* pNetObject = m_net_->GetNet()->GetNetObject(sock);
        if (pNetObject) {
            pNetObject->SetClientID(xClientIdent);
        }

        //NetObject* pNetObject = m_net_->GetNet()->GetNetObject(sock);
        pNetObject->SetConnectKeyState(1);
        pNetObject->SetSecurityKey(req.key());
        //this net-object bind a user's account
        pNetObject->SetAccount(guid.ToString());
        pNetObject->SetUserID(guid);
        rpc::AckConnectProxy ack;
        //dout << guid.ToString() << " 连接成功!\n";
        ack.set_code(0);
        m_net_->SendMsgPB(rpc::ProxyRPC::ACK_CONNECT_PROXY, ack, sock);

        mxClientIdent.AddElement(xClientIdent, std::shared_ptr<socket_t>(new socket_t(sock)));
    } else {
        //if verify failed then close this connect
        m_net_->GetNet()->CloseNetObject(sock);
    }
}

}