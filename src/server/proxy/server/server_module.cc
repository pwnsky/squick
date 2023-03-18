#include "plugin.h"

#include "server_module.h"
namespace proxy::server {
bool ServerModule::Start()
{
	this->pPluginManager->SetAppType(SQUICK_SERVER_TYPES::SQUICK_ST_PROXY);

	m_pNetModule = pPluginManager->FindModule<INetModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pClassModule = pPluginManager->FindModule<IClassModule>();
	m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pProxyToWorldModule = pPluginManager->FindModule<client::IWorldModule>();
	m_pSecurityModule = pPluginManager->FindModule<ISecurityModule>();
	m_pWsModule = pPluginManager->FindModule<IWSModule>();
    m_pThreadPoolModule = pPluginManager->FindModule<IThreadPoolModule>();

    return true;
}

bool ServerModule::AfterStart()
{
	//m_pNetModule->AddReceiveCallBack(SquickStruct::ProxyRPC::REQ_LOGIN, this, &ServerModule::OnReqLogin);
    //m_pNetModule->AddReceiveCallBack(SquickStruct::GameLobbyRPC::REQ_ENTER, this, &ServerModule::OnReqEnterGameServer);
	m_pNetModule->AddReceiveCallBack(this, &ServerModule::OnOtherMessage);
    
    // 绑定Call back以及转发去向
	m_pNetModule->AddEventCallBack(this, &ServerModule::OnSocketClientEvent);
	m_pNetModule->ExpandBufferSize(1024*1024*2);

    SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass)
    {
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
		for (int i = 0; i < strIdList.size(); ++i)
		{
			const std::string& strId = strIdList[i];

            const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_PROXY && pPluginManager->GetAppID() == serverID)
            {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());
                //const std::string& name = m_pElementModule->GetPropertyString(strId, SquickProtocol::Server::ID());
                //const std::string& ip = m_pElementModule->GetPropertyString(strId, SquickProtocol::Server::IP());
                
                // 绑定端口
                int nRet = m_pNetModule->Startialization(maxConnect, nPort, nCpus);
                if (nRet < 0)
                {
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

bool ServerModule::Destory()
{
    return true;
}

bool ServerModule::Update()
{
	return true;
}

void ServerModule::OnOtherMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
	if (!pNetObject || pNetObject->GetConnectKeyState() <= 0 || pNetObject->GetGameID() <= 0)
	{
		//state error
		return;
	}

	std::string strMsgData = m_pSecurityModule->DecodeMsg(pNetObject->GetAccount(), pNetObject->GetSecurityKey(), msgID, msg, len);
	if (strMsgData.empty())
	{
		//decode failed
		m_pLogModule->LogError(Guid(0, sockIndex), "DecodeMsg failed", __FUNCTION__, __LINE__);
		return;
	}

	SquickStruct::MsgBase xMsg;
	if (!xMsg.ParseFromString(strMsgData))
	{
		char szData[MAX_PATH] = { 0 };
		sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msgID);

		m_pLogModule->LogError(Guid(0, sockIndex), szData, __FUNCTION__, __LINE__);
		return;
	}


	//real user id
	*xMsg.mutable_player_id() = INetModule::StructToProtobuf(pNetObject->GetUserID());


	std::string msgData;
	if (!xMsg.SerializeToString(&msgData))
	{
		return;
	}

	if (xMsg.has_hash_ident())
	{
		//special for distributed
		if (!pNetObject->GetHashIdentID().IsNull())
		{
			m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_GAME, pNetObject->GetHashIdentID().ToString(), msgID, msgData);
		}
		else
		{
			Guid xHashIdent = INetModule::ProtobufToStruct(xMsg.hash_ident());
			m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_GAME, xHashIdent.ToString(), msgID, msgData);
		}
	}
	else
	{
        if (msgID >= 50000)
        {
			m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, pNetObject->GetUserID().ToString(), msgID, msgData);
        }
        else
        {
		    m_pNetClientModule->SendByServerIDWithOutHead(pNetObject->GetGameID(), msgID, msgData);
        }
        
	}
}

void ServerModule::OnReqLogin(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
    Guid nPlayerID;
    /*
    SquickStruct::ReqLogin xMsg;
    if (!m_pNetModule->ReceivePB( msgID, msg, len, xMsg, nPlayerID))
    {
        return;
    }
    
    // 验证Token
	bool bRet = m_pSecurityModule->VerifySecurityKey(xMsg.account(), xMsg.token());
    //bool bRet = m_pProxyToWorldModule->VerifyConnectData(xMsg.account(), xMsg.security_code());
    if (bRet)
    {
        NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
        if (pNetObject)
        {
            //this net-object verify successful and set state as true
            pNetObject->SetConnectKeyState(1);
			pNetObject->SetSecurityKey(xMsg.token());

            //this net-object bind a user's account
            pNetObject->SetAccount(xMsg.account());

            
            SquickStruct::AckLogin xSendMsg;
            dout << xMsg.account() << " 登录成功!\n";
            xSendMsg.set_code(0);
            xSendMsg.set_token("abcd_token");
            *xSendMsg.mutable_guid() = INetModule::StructToProtobuf(pNetObject->GetClientID());
			m_pNetModule->SendMsgPB(SquickStruct::ProxyRPC::ACK_LOGIN, xSendMsg, sockIndex);
            
        }
    } else {
        //if verify failed then close this connect
		m_pNetModule->GetNet()->CloseNetObject(sockIndex);
    }*/
}

void ServerModule::OnSocketClientEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet)
{
    if (eEvent & SQUICK_NET_EVENT_EOF)
    {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
        OnClientDisconnect(sockIndex);
    }
    else if (eEvent & SQUICK_NET_EVENT_ERROR)
    {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
        OnClientDisconnect(sockIndex);
    }
    else if (eEvent & SQUICK_NET_EVENT_TIMEOUT)
    {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
        OnClientDisconnect(sockIndex);
    }
    else  if (eEvent & SQUICK_NET_EVENT_CONNECTED)
    {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
        OnClientConnected(sockIndex);
    }
}

void ServerModule::OnClientDisconnect(const SQUICK_SOCKET nAddress)
{
    NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(nAddress);
    if (pNetObject)
    {
        int nGameID = pNetObject->GetGameID();
        if (nGameID > 0) {
            //when a net-object bind a account then tell that game-server
            if (!pNetObject->GetUserID().IsNull()) {
                SquickStruct::ReqLeave xData;
                SquickStruct::MsgBase xMsg;

				//real user id
                *xMsg.mutable_player_id() = INetModule::StructToProtobuf(pNetObject->GetUserID());

                if (!xData.SerializeToString(xMsg.mutable_msg_data())) {
                    return;
                }

                std::string msg;
                if (!xMsg.SerializeToString(&msg))
                {
                    return;
                }

				m_pNetClientModule->SendByServerIDWithOutHead(nGameID, SquickStruct::GameLobbyRPC::REQ_LEAVE, msg);
            }
        }
        mxClientIdent.RemoveElement(pNetObject->GetClientID());
    }
}

// 选择服务器
bool ServerModule::SelectGameServer(int sockIndex)
{
	NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
	if (!pNetObject) {
		return false;
	}

    // 查找最小负载的服务器
    //actually, if you want the game server working with a good performance then we need to find the game server with lowest workload
	int nWorkload = 999999;
	int nGameID = 0;
    MapEx<int, ConnectData>& xServerList = m_pNetClientModule->GetServerList();
    ConnectData* pGameData = xServerList.FirstNude();
    while (pGameData) {
        if (ConnectDataState::NORMAL == pGameData->eState
            && SQUICK_SERVER_TYPES::SQUICK_ST_GAME == pGameData->eServerType) {
			if (pGameData->nWorkLoad < nWorkload) {
				nWorkload = pGameData->nWorkLoad;
				nGameID = pGameData->nGameID;
			}
        }
        pGameData = xServerList.NextNude();
    }

	if (nGameID > 0)
	{
		pNetObject->SetGameID(nGameID);
		return true;
	}

    // 如果服务器比较繁忙，给客户端响应繁忙
    return false;

}

int ServerModule::Transport(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
    SquickStruct::MsgBase xMsg;
    if (!xMsg.ParseFromArray(msg, len))
    {
        char szData[MAX_PATH] = { 0 };
        sprintf(szData, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msgID);

        return false;
    }

    // broadcast many palyers
    for (int i = 0; i < xMsg.player_client_list_size(); ++i)
    {
        SQUICK_SHARE_PTR<SQUICK_SOCKET> pFD = mxClientIdent.GetElement(INetModule::ProtobufToStruct(xMsg.player_client_list(i)));
        if (pFD)
        {
            if (xMsg.has_hash_ident())
            {
                NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(*pFD);
                if (pNetObject)
                {
                    pNetObject->SetHashIdentID(INetModule::ProtobufToStruct(xMsg.hash_ident()));
                }
            }
			m_pNetModule->SendMsgWithOutHead(msgID, std::string(msg, len), *pFD);
        }
    }

    //send message to one player
    if (xMsg.player_client_list_size() <= 0)
    {
		Guid xClientIdent = INetModule::ProtobufToStruct(xMsg.player_id());
        SQUICK_SHARE_PTR<SQUICK_SOCKET> pFD = mxClientIdent.GetElement(xClientIdent);
        if (pFD)
        {
            if (xMsg.has_hash_ident())
            {
                NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(*pFD);
                if (pNetObject)
                {
                    pNetObject->SetHashIdentID(INetModule::ProtobufToStruct(xMsg.hash_ident()));
                }
            }

			m_pNetModule->SendMsgWithOutHead(msgID, std::string(msg, len), *pFD);
        }
		else if(xClientIdent.IsNull())
		{
			//send this msessage to all clientss
			m_pNetModule->GetNet()->SendMsgToAllClientWithOutHead(msgID, msg, len);
		}
		//pFD is empty means end of connection, no need to send message to this client any more. And,
		//we should never send a message that specified to a player to all clients here.
		else
		{
		}
    }

    return true;
}

void ServerModule::OnClientConnected(const SQUICK_SOCKET nAddress)
{
    std::cout << "Client Connected.... \n";
	//bind client'id with socket id
    Guid xClientIdent = m_pKernelModule->CreateGUID();
    NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(nAddress);
    if (pNetObject)
    {
        pNetObject->SetClientID(xClientIdent);
    }

    mxClientIdent.AddElement(xClientIdent, SQUICK_SHARE_PTR<SQUICK_SOCKET>(new SQUICK_SOCKET(nAddress)));
}

// 请求进入游戏
void ServerModule::OnReqEnterGameServer(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
    dout << "请求进入游戏\n";

    SelectGameServer(sockIndex);

	NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
	if (!pNetObject)
	{
		return;
	}

	std::string strMsgData = m_pSecurityModule->DecodeMsg(pNetObject->GetAccount(), pNetObject->GetSecurityKey(), msgID, msg, len);
	if (strMsgData.empty())
	{
		//decode failed
		return;
	}

    Guid nPlayerID;//no value
    SquickStruct::ReqEnter xData;
    if (!m_pNetModule->ReceivePB( msgID, msg, len, xData, nPlayerID))
    {
        return;
    }
    
    SQUICK_SHARE_PTR<ConnectData> pServerData = m_pNetClientModule->GetServerNetInfo(pNetObject->GetGameID());
    if (pServerData && ConnectDataState::NORMAL == pServerData->eState)
    {
        if (pNetObject->GetConnectKeyState() > 0 )
        {
            SquickStruct::MsgBase xMsg;
            if (!xData.SerializeToString(xMsg.mutable_msg_data()))
            {
                return;
            }

			//clientid
            xMsg.mutable_player_id()->CopyFrom(INetModule::StructToProtobuf(pNetObject->GetClientID()));
            std::string msg;
            if (!xMsg.SerializeToString(&msg))
            {
                return;
            }
			m_pNetClientModule->SendByServerIDWithOutHead(pNetObject->GetGameID(), SquickStruct::GameLobbyRPC::REQ_ENTER, msg);
        }
    }
}


int ServerModule::EnterGameSuccessEvent(const Guid xClientID, const Guid xPlayerID)
{
    SQUICK_SHARE_PTR<SQUICK_SOCKET> pFD = mxClientIdent.GetElement(xClientID);
    if (pFD)
    {
        NetObject* pNetObeject = m_pNetModule->GetNet()->GetNetObject(*pFD);
        if (pNetObeject)
        {
            pNetObeject->SetUserID(xPlayerID);
        }
    }
    return 0;
}

}