

#include "server_module.h"
#include <squick/plugin/lua/export.h>
#include <squick/struct/struct.h>
#include <third_party/nlohmann/json.hpp>
namespace gameplay_manager::server {
bool ServerModule::Start() {
    this->pPluginManager->SetAppType(SQUICK_SERVER_TYPES::SQUICK_ST_GAMEPLAY_MANAGER);
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pSecurityModule = pPluginManager->FindModule<ISecurityModule>();
    m_pWsModule = pPluginManager->FindModule<IWSModule>();
    return true;
}

bool ServerModule::Destory() { return true; }

bool ServerModule::AfterStart() {
    // PVP校验连接Key,校验成功后，之后的数据直接转发到Game服务器上
    m_pNetModule->AddReceiveCallBack(SquickStruct::REQ_GAMEPLAY_CONNECT_GAME_SERVER, this, &ServerModule::OnReqConnectGameServer);
    m_pNetModule->AddReceiveCallBack(this, &ServerModule::OnOtherMessage);

    // 创建PVP管理服务器, 用于接收PVP服务器的数据
    SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];
            const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_GAMEPLAY_MANAGER && pPluginManager->GetAppID() == serverID) {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());

                // mAreaID = m_pElementModule->GetPropertyInt32(strId, SquickProtocol::Server::Area());
                // const std::string& name = m_pElementModule->GetPropertyString(strId, SquickProtocol::Server::ID());
                // const std::string& ip = m_pElementModule->GetPropertyString(strId, SquickProtocol::Server::IP());
                int nRet = m_pNetModule->Startialization(maxConnect, nPort, nCpus);
                if (nRet < 0) {
                    std::ostringstream strLog;
                    strLog << "Cannot init server net, Port = " << nPort;
                    m_pLogModule->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                    NFASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
                    exit(0);
                }
            }
        }
    }
    return true;
}

// Gameplay服务器请求连接 Game 服务器
void ServerModule::OnReqConnectGameServer(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    dout << "--- Gameplay服务器与Game服务器建立连接 " << std::endl;
    Guid nPlayerID;
    SquickStruct::ReqGameplayConnectGameServer xMsg;
    if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, nPlayerID)) {
        return;
    }

    // 验证Token
    //bool bRet = m_pSecurityModule->VerifySecurityKey(xMsg.id(), xMsg.security_code());
    // bool bRet = m_pProxyToWorldModule->VerifyConnectData(xMsg.account(), xMsg.security_code());
    auto bRet = true;
    if (!bRet) {
        // if verify failed then close this connect
        m_pNetModule->GetNet()->CloseNetObject(sockIndex);
        return;
    }

    NetObject *pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
    if (!pNetObject) {
        return;
    }

    // this net-object verify successful and set state as true
    pNetObject->SetConnectKeyState(1);

    //pNetObject->SetSecurityKey(xMsg.security_code());

    // this net-object bind a account
    //pNetObject->SetAccount(xMsg.id());

    SquickStruct::AckGameplayConnectGameServer xSendMsg;
    xSendMsg.set_code(0);
    m_pNetModule->SendMsgPB(SquickStruct::ACK_GAMEPLAY_CONNECT_GAME_SERVER, xSendMsg, sockIndex);

    SQUICK_SHARE_PTR<ConnectData> pServerData = m_pNetClientModule->GetServerNetInfo(xMsg.game_id());
    if (pServerData && ConnectDataState::NORMAL == pServerData->eState) {
        // 设置可发送到game server
        std::cout << " Set Game ID: " << xMsg.game_id() << " status: " << pServerData->eState << std::endl;
        pNetObject->SetGameID(xMsg.game_id());
    } else {
        std::cout << " 查找存在问题: " << pServerData << std::endl;
        if (pServerData) {
            std::cout << " status: " << pServerData->eState << std::endl;
        }
    }
}

// 来自PVP服务器消息，根据校验连接成功PVP服务器数据包转发达到Game服务器
void ServerModule::OnOtherMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    dout << "转发消息\n";
    NetObject *pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
    if (!pNetObject || pNetObject->GetConnectKeyState() <= 0 || pNetObject->GetGameID() <= 0) {
        // state error
        return;
    }

    // 校验
    std::string strMsgData = m_pSecurityModule->DecodeMsg(pNetObject->GetAccount(), pNetObject->GetSecurityKey(), msgID, msg, len);
    if (strMsgData.empty()) {
        // decode failed
        m_pLogModule->LogError(Guid(0, sockIndex), "DecodeMsg failed", __FUNCTION__, __LINE__);
        return;
    }

    SquickStruct::MsgBase xMsg;
    if (!xMsg.ParseFromString(strMsgData)) {
        char szData[MAX_PATH] = {0};
        sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msgID);

        m_pLogModule->LogError(Guid(0, sockIndex), szData, __FUNCTION__, __LINE__);
        return;
    }

    // real pvp id
    *xMsg.mutable_player_id() = INetModule::StructToProtobuf(pNetObject->GetUserID());

    std::string msgData;
    if (!xMsg.SerializeToString(&msgData)) {
        return;
    }
    dout << "PVP Manger 转发消息\n";
    if (xMsg.has_hash_ident()) {
        // special for distributed
        if (!pNetObject->GetHashIdentID().IsNull()) {
            m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_GAME, pNetObject->GetHashIdentID().ToString(), msgID, msgData);
        } else {
            Guid xHashIdent = INetModule::ProtobufToStruct(xMsg.hash_ident());
            m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_GAME, xHashIdent.ToString(), msgID, msgData);
        }
    } else {
        if (msgID >= 50000) {
            m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, pNetObject->GetUserID().ToString(), msgID, msgData);
        } else
            dout << "PVP Manger 转发给 Game Server\n";
        { m_pNetClientModule->SendByServerIDWithOutHead(pNetObject->GetGameID(), msgID, msgData); }
    }
}

// PVP 服务端断开连接
void ServerModule::OnClientDisconnect(const SQUICK_SOCKET nAddress) {
    NetObject *pNetObject = m_pNetModule->GetNet()->GetNetObject(nAddress);
    if (pNetObject) {
        int nGameID = pNetObject->GetGameID();
        if (nGameID > 0) {
            // when a net-object bind a account then tell that game-server
            if (!pNetObject->GetUserID().IsNull()) {
                SquickStruct::AckGameplayDestroyed xData;
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

                // 也告知一下Game服务器
                m_pNetClientModule->SendByServerIDWithOutHead(nGameID, SquickStruct::GameplayManagerRPC::REQ_GAMEPLAY_DESTROY, msg);
            }
        }

        mxClientIdent.RemoveElement(pNetObject->GetClientID());
    }
}

// 转发给PVP服务器
bool ServerModule::Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    SquickStruct::MsgBase xMsg;
    if (!xMsg.ParseFromArray(msg, len)) {
        char szData[MAX_PATH] = {0};
        sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msgID);

        return false;
    }

    // broadcast many palyers
    for (int i = 0; i < xMsg.player_client_list_size(); ++i) {
        SQUICK_SHARE_PTR<SQUICK_SOCKET> pFD = mxClientIdent.GetElement(INetModule::ProtobufToStruct(xMsg.player_client_list(i)));
        if (pFD) {
            if (xMsg.has_hash_ident()) {
                NetObject *pNetObject = m_pNetModule->GetNet()->GetNetObject(*pFD);
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
                NetObject *pNetObject = m_pNetModule->GetNet()->GetNetObject(*pFD);
                if (pNetObject) {
                    pNetObject->SetHashIdentID(INetModule::ProtobufToStruct(xMsg.hash_ident()));
                }
            }

            m_pNetModule->SendMsgWithOutHead(msgID, std::string(msg, len), *pFD);
        } else if (xClientIdent.IsNull()) {
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

bool ServerModule::Update() { return true; }

} // namespace gameplay_manager::server