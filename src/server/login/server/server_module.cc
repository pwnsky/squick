
#include "plugin.h"
#include "server_module.h"

bool LoginNet_ServerModule::Start()
{
	this->pPluginManager->SetAppType(SQUICK_SERVER_TYPES::SQUICK_ST_LOGIN);

	m_pNetModule = pPluginManager->FindModule<INetModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();
	m_pClassModule = pPluginManager->FindModule<IClassModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
	m_pLoginToMasterModule = pPluginManager->FindModule<ILoginToMasterModule>();
	m_pThreadPoolModule = pPluginManager->FindModule<IThreadPoolModule>();

	return true;
}

bool LoginNet_ServerModule::Destory()
{
	return true;
}

bool LoginNet_ServerModule::BeforeDestory()
{
	return true;
}

bool LoginNet_ServerModule::AfterStart()
{
	m_pNetModule->AddReceiveCallBack(SquickStruct::STS_HEART_BEAT, this, &LoginNet_ServerModule::OnHeartBeat);
	m_pNetModule->AddReceiveCallBack(SquickStruct::REQ_LOGIN, this, &LoginNet_ServerModule::OnLoginProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::REQ_LOGOUT, this, &LoginNet_ServerModule::OnLogOut);
	m_pNetModule->AddReceiveCallBack(this, &LoginNet_ServerModule::InvalidMessage);
	m_pNetModule->AddEventCallBack(this, &LoginNet_ServerModule::OnSocketClientEvent);
	m_pNetModule->ExpandBufferSize();

	SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
		for (int i = 0; i < strIdList.size(); ++i)
		{
			const std::string& strId = strIdList[i];

			const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
			const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
			if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_LOGIN && pPluginManager->GetAppID() == serverID)
			{
				const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
				const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
				const int nCpus = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());

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


bool LoginNet_ServerModule::Update()
{
	return true;
}

void LoginNet_ServerModule::OnClientConnected(const SQUICK_SOCKET nAddress)
{
	NetObject* pObject = m_pNetModule->GetNet()->GetNetObject(nAddress);
	if (pObject)
	{
		Guid xIdent = m_pKernelModule->CreateGUID();
		pObject->SetClientID(xIdent);
		mxClientIdent.AddElement(xIdent, SQUICK_SHARE_PTR<SQUICK_SOCKET>(SQUICK_NEW SQUICK_SOCKET(nAddress)));
	}
}

void LoginNet_ServerModule::OnClientDisconnect(const SQUICK_SOCKET nAddress)
{
	NetObject* pObject = m_pNetModule->GetNet()->GetNetObject(nAddress);
	if (pObject)
	{
		Guid xIdent = pObject->GetClientID();
		mxClientIdent.RemoveElement(xIdent);
	}
}

void LoginNet_ServerModule::OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ReqLogin xMsg;
	if (!m_pNetModule->ReceivePB( msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	NetObject* pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
	if (pNetObject)
	{
		if (pNetObject->GetConnectKeyState() == 0)
		{
			std::ostringstream strLog;
			strLog << "登录, Account = " << xMsg.account() << " Password = " << xMsg.password();
			std::cout << " 登录: " << xMsg.account() << " / " << xMsg.password() << std::endl;
			//Normally, you could check the account and password is correct or not, but for our situation, it will correct by default as here is the tutorial code.
			int loginResult = 0;//0 means successful, else means error code from account platform.
			if (0 != loginResult)
			{
				strLog << "Check password failed, Account = " << xMsg.account() << " Password = " << xMsg.password();
				m_pLogModule->LogError(Guid(0, sockIndex), strLog, __FUNCTION__, __LINE__);

				SquickStruct::AckLogin xMsg;
				xMsg.set_code(0);

				//m_pNetModule->SendMsgPB(SquickStruct::EGameMsgID::ACK_LOGIN, xMsg, sockIndex);
				return;
			}

			pNetObject->SetConnectKeyState(1);
			pNetObject->SetAccount(xMsg.account());

			SquickStruct::AckLogin xData;
			xData.set_code(0);

			//The login server responds the login result to the player by sock id.
			m_pNetModule->SendMsgPB(SquickStruct::ProxyRPC::ACK_LOGIN, xData, sockIndex);

			m_pLogModule->LogInfo(Guid(0, sockIndex), "Login succeeded :", xMsg.account().c_str());
		}
	}
}

void LoginNet_ServerModule::OnSelectWorldProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
}

void LoginNet_ServerModule::OnSocketClientEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet)
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

void LoginNet_ServerModule::SynWorldToClient(const SQUICK_SOCKET nFD)
{
	SquickStruct::AckServerList xData;

	MapEx<int, SquickStruct::ServerInfoReport>& xWorldMap = m_pLoginToMasterModule->GetWorldMap();
	SquickStruct::ServerInfoReport* pWorldData = xWorldMap.FirstNude();
	while (pWorldData)
	{
		SquickStruct::ServerInfo* pServerInfo = xData.add_info();

		pServerInfo->set_name(pWorldData->server_name());
		pServerInfo->set_status(pWorldData->server_state());
		pServerInfo->set_server_id(pWorldData->server_id());
		pServerInfo->set_wait_count(0);

		pWorldData = xWorldMap.NextNude();
	}
}

void LoginNet_ServerModule::OnViewWorldProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
}

void LoginNet_ServerModule::OnHeartBeat(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
}

void LoginNet_ServerModule::OnLogOut(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
}

void LoginNet_ServerModule::InvalidMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	printf("Net || umsgID=%d\n", msgID);
}
