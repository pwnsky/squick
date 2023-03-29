
#include "logic_module.h"
#include "plugin.h"

namespace proxy::logic {

bool LogicModule::Start() { return true; }

bool LogicModule::Destory() { return true; }

bool LogicModule::Update() { return true; }

bool LogicModule::AfterStart() {
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();

    m_pNetModule->AddReceiveCallBack(this, &LogicModule::OnOtherMessage);
    m_pNetModule->AddReceiveCallBack(SquickStruct::ProxyRPC::REQ_HEARTBEAT, this, &LogicModule::OnHeartbeat);
    m_pNetModule->AddReceiveCallBack(SquickStruct::ProxyRPC::REQ_CONNECT_PROXY, this, &LogicModule::OnReqConnect);
    m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ENTER, this, &LogicModule::OnReqEnterGameServer);
    
    return true;
}

void LogicModule::OnClientDisconnect(const SQUICK_SOCKET nAddress) {
    NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(nAddress);
    if (pNetObject) {
        int nGameID = pNetObject->GetGameID();
        if (nGameID > 0) {
            // when a net-object bind a account then tell that game-server
            if (!pNetObject->GetUserID().IsNull()) {
                SquickStruct::ReqLeave xData;
                SquickStruct::MsgBase xMsg;

                // real user id
                *xMsg.mutable_player_id() = INetModule::StructToProtobuf(pNetObject->GetUserID());

                if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
                    return;
                }

                std::string msg;
                if (!xMsg.SerializeToString(&msg)) {
                    return;
                }

                m_pNetClientModule->SendByServerIDWithOutHead(nGameID, SquickStruct::GameLobbyRPC::REQ_LEAVE, msg);
            }
        }
        mxClientIdent.RemoveElement(pNetObject->GetClientID());
    }
}



int LogicModule::Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len) {
    SquickStruct::MsgBase xMsg;
    if (!xMsg.ParseFromArray(msg, len)) {
        char szData[MAX_PATH] = { 0 };
        sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msgID);
        return false;
    }

    // broadcast many palyers
    for (int i = 0; i < xMsg.player_client_list_size(); ++i) {
        SQUICK_SHARE_PTR<SQUICK_SOCKET> pFD = mxClientIdent.GetElement(INetModule::ProtobufToStruct(xMsg.player_client_list(i)));
        if (pFD) {
            if (xMsg.has_hash_ident()) {
                NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(*pFD);
                if (pNetObject) {
                    pNetObject->SetHashIdentID(INetModule::ProtobufToStruct(xMsg.hash_ident()));
                }
            }
            m_pNetModule->SendMsgWithOutHead(msgID, std::string(msg, len), *pFD);
        }
    }

    // send message to one player
    if (xMsg.player_client_list_size() <= 0) {
        Guid xClientIdent = INetModule::ProtobufToStruct(xMsg.player_id());
        SQUICK_SHARE_PTR<SQUICK_SOCKET> pFD = mxClientIdent.GetElement(xClientIdent);
        if (pFD) {
            if (xMsg.has_hash_ident()) {
                NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(*pFD);
                if (pNetObject) {
                    pNetObject->SetHashIdentID(INetModule::ProtobufToStruct(xMsg.hash_ident()));
                }
            }

            m_pNetModule->SendMsgWithOutHead(msgID, std::string(msg, len), *pFD);
        }
        else if (xClientIdent.IsNull()) {
            // send this msessage to all clientss
            m_pNetModule->GetNet()->SendMsgToAllClientWithOutHead(msgID, msg, len);
        }
        // pFD is empty means end of connection, no need to send message to this client any more. And,
        // we should never send a message that specified to a player to all clients here.
        else {
        }
    }
    return true;
}

void LogicModule::OnClientConnected(const SQUICK_SOCKET nAddress) {
    std::cout << "Client Connected.... \n";
}

int LogicModule::EnterGameSuccessEvent(const Guid xClientID, const Guid xPlayerID) {
    SQUICK_SHARE_PTR<SQUICK_SOCKET> pFD = mxClientIdent.GetElement(xClientID);
    if (pFD) {
        NetObject* pNetObeject = m_pNetModule->GetNet()->GetNetObject(*pFD);
        if (pNetObeject) {
            pNetObeject->SetUserID(xPlayerID);
        }
    }
    return 0;
}


void LogicModule::OnOtherMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len) {
    NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
    if (!pNetObject || pNetObject->GetConnectKeyState() <= 0 || pNetObject->GetGameID() <= 0) {
        // state error
        return;
    }

    SquickStruct::MsgBase xMsg;
    if (!xMsg.ParseFromString(std::string(msg, len))) {
        char szData[MAX_PATH] = { 0 };
        sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msgID);

        m_pLogModule->LogError(Guid(0, sockIndex), szData, __FUNCTION__, __LINE__);
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
            m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_GAME, pNetObject->GetHashIdentID().ToString(), msgID, msgData);
        }
        else {
            Guid xHashIdent = INetModule::ProtobufToStruct(xMsg.hash_ident());
            m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_GAME, xHashIdent.ToString(), msgID, msgData);
        }
    }
    else {
        if (msgID >= 50000) {
            m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, pNetObject->GetUserID().ToString(), msgID, msgData);
        }
        else {
            m_pNetClientModule->SendByServerIDWithOutHead(pNetObject->GetGameID(), msgID, msgData);
        }
    }
}


void LogicModule::OnHeartbeat(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    std::string msgData(msg, len);
    m_pNetModule->SendMsgWithOutHead(SquickStruct::ProxyRPC::ACK_HEARTBEAT, msgData, sockIndex);
}


// 选择服务器
bool LogicModule::SelectGameServer(int sock) {
    NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sock);
    if (!pNetObject) {
        return false;
    }

    // 查找最小负载的服务器
    // actually, if you want the game server working with a good performance then we need to find the game server with lowest workload
    int nWorkload = 999999;
    int nGameID = 0;
    MapEx<int, ConnectData>& xServerList = m_pNetClientModule->GetServerList();
    ConnectData* pGameData = xServerList.FirstNude();
    while (pGameData) {
        if (ConnectDataState::NORMAL == pGameData->eState && SQUICK_SERVER_TYPES::SQUICK_ST_GAME == pGameData->eServerType) {
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
void LogicModule::OnReqEnterGameServer(const SQUICK_SOCKET sock, const int msg_id, const char* msg, const uint32_t len) {
    dout << "请求进入游戏\n";

    SelectGameServer(sock);

    NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sock);
    if (!pNetObject) {
        return;
    }

    Guid nPlayerID; // no value
    SquickStruct::ReqEnter xData;
    if (!m_pNetModule->ReceivePB(msg_id, msg, len, xData, nPlayerID)) {
        return;
    }

    SQUICK_SHARE_PTR<ConnectData> pServerData = m_pNetClientModule->GetServerNetInfo(pNetObject->GetGameID());
    if (pServerData && ConnectDataState::NORMAL == pServerData->eState) {
        if (pNetObject->GetConnectKeyState() > 0) {
            SquickStruct::MsgBase xMsg;
            if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
                return;
            }

            // clientid
            xMsg.mutable_player_id()->CopyFrom(INetModule::StructToProtobuf(pNetObject->GetClientID()));
            std::string msg;
            if (!xMsg.SerializeToString(&msg)) {
                return;
            }

            m_pNetClientModule->SendByServerIDWithOutHead(pNetObject->GetGameID(), SquickStruct::GameLobbyRPC::REQ_ENTER, msg);
        }
    }
}


void LogicModule::OnReqConnect(const SQUICK_SOCKET sock, const int msg_id, const char* msg, const uint32_t len) {
    Guid nPlayerID;
    
    SquickStruct::ReqConnectProxy req;
    if (!m_pNetModule->ReceivePB(msg_id, msg, len, req, nPlayerID))
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
        NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sock);
        if (pNetObject) {
            pNetObject->SetClientID(xClientIdent);
        }

        //NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sock);
        pNetObject->SetConnectKeyState(1);
        pNetObject->SetSecurityKey(req.key());
        //this net-object bind a user's account
        pNetObject->SetAccount(guid.ToString());
        pNetObject->SetUserID(guid);
        SquickStruct::AckConnectProxy ack;
        //dout << guid.ToString() << " 连接成功!\n";
        ack.set_code(0);
        m_pNetModule->SendMsgPB(SquickStruct::ProxyRPC::ACK_CONNECT_PROXY, ack, sock);

        mxClientIdent.AddElement(xClientIdent, SQUICK_SHARE_PTR<SQUICK_SOCKET>(new SQUICK_SOCKET(sock)));
    } else {
        //if verify failed then close this connect
        m_pNetModule->GetNet()->CloseNetObject(sock);
    }
}

}