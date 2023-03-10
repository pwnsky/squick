
#include "plugin.h"
#include "server_module.h"
#include <third_party/nlohmann/json.hpp>
MasterNet_ServerModule::~MasterNet_ServerModule()
{

}

bool MasterNet_ServerModule::Start()
{
	this->pPluginManager->SetAppType(SQUICK_SERVER_TYPES::SQUICK_ST_MASTER);

	m_pNetModule = pPluginManager->FindModule<INetModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();
	m_pClassModule = pPluginManager->FindModule<IClassModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();

	return true;
}

bool MasterNet_ServerModule::Destory()
{
	return true;
}

void MasterNet_ServerModule::OnWorldRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ServerInfoReportList xMsg;
	if (!m_pNetModule->ReceivePB( msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
		SQUICK_SHARE_PTR<ServerData> pServerData = mWorldMap.GetElement(xData.server_id());
		if (!pServerData)
		{
			pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
			mWorldMap.AddElement(xData.server_id(), pServerData);
		}

		pServerData->nFD = sockIndex;
		*(pServerData->pData) = xData;

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "WorldRegistered");
	}

	SynWorldToLoginAndWorld();
}

void MasterNet_ServerModule::OnWorldUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ServerInfoReportList xMsg;
	if (!m_pNetModule->ReceivePB( msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
		mWorldMap.RemoveElement(xData.server_id());


		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "WorldUnRegistered");
	}

	SynWorldToLoginAndWorld();
}

void MasterNet_ServerModule::OnRefreshWorldInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ServerInfoReportList xMsg;
	if (!m_pNetModule->ReceivePB( msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
		SQUICK_SHARE_PTR<ServerData> pServerData = mWorldMap.GetElement(xData.server_id());
		if (!pServerData)
		{
			pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
			mWorldMap.AddElement(xData.server_id(), pServerData);
		}

		pServerData->nFD = sockIndex;
		*(pServerData->pData) = xData;

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "RefreshWorldInfo");

	}

	SynWorldToLoginAndWorld();
}

void MasterNet_ServerModule::OnLoginRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ServerInfoReportList xMsg;
	if (!m_pNetModule->ReceivePB( msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
		SQUICK_SHARE_PTR<ServerData> pServerData = mLoginMap.GetElement(xData.server_id());
		if (!pServerData)
		{
			pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
			mLoginMap.AddElement(xData.server_id(), pServerData);
		}

		pServerData->nFD = sockIndex;
		*(pServerData->pData) = xData;

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "LoginRegistered");
	}

	SynWorldToLoginAndWorld();
}

void MasterNet_ServerModule::OnLoginUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ServerInfoReportList xMsg;
	if (!m_pNetModule->ReceivePB( msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);

		mLoginMap.RemoveElement(xData.server_id());

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "LoginUnRegistered");

	}
}

void MasterNet_ServerModule::OnRefreshLoginInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ServerInfoReportList xMsg;
	if (!m_pNetModule->ReceivePB( msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
		SQUICK_SHARE_PTR<ServerData> pServerData = mLoginMap.GetElement(xData.server_id());
		if (!pServerData)
		{
			pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
			mLoginMap.AddElement(xData.server_id(), pServerData);
		}

		pServerData->nFD = sockIndex;
		*(pServerData->pData) = xData;

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "RefreshLoginInfo");

	}
}


bool MasterNet_ServerModule::Update()
{
	//LogGameServer();
	return true;
}

bool MasterNet_ServerModule::AfterStart()
{
	m_pNetModule->AddReceiveCallBack(SquickStruct::ServerRPC::STS_HEART_BEAT, this, &MasterNet_ServerModule::OnHeartBeat);
	m_pNetModule->AddReceiveCallBack(SquickStruct::ServerRPC::WORLD_TO_MASTER_REGISTERED, this, &MasterNet_ServerModule::OnWorldRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::ServerRPC::WORLD_TO_MASTER_UNREGISTERED, this, &MasterNet_ServerModule::OnWorldUnRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::ServerRPC::WORLD_TO_MASTER_REFRESH, this, &MasterNet_ServerModule::OnRefreshWorldInfoProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::ServerRPC::LOGIN_TO_MASTER_REGISTERED, this, &MasterNet_ServerModule::OnLoginRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::ServerRPC::LOGIN_TO_MASTER_UNREGISTERED, this, &MasterNet_ServerModule::OnLoginUnRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::ServerRPC::LOGIN_TO_MASTER_REFRESH, this, &MasterNet_ServerModule::OnRefreshLoginInfoProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::ServerRPC::STS_SERVER_REPORT, this, &MasterNet_ServerModule::OnServerReport);

	m_pNetModule->AddReceiveCallBack(this, &MasterNet_ServerModule::InvalidMessage);

	m_pNetModule->AddEventCallBack(this, &MasterNet_ServerModule::OnSocketEvent);
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
			if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_MASTER && pPluginManager->GetAppID() == serverID)
			{
				const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
				const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
				const int nCpus = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());
				const std::string& name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
				const std::string& ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

				int nRet = m_pNetModule->Startialization(maxConnect, nPort, nCpus);
				if (nRet < 0)
				{
					std::ostringstream strLog;
					strLog << "Cannot init server net, Port = " << nPort;
					m_pLogModule->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
					NFASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
					exit(0);
				}
				SquickStruct::ServerInfoReport reqMsg;

				reqMsg.set_server_id(serverID);
				reqMsg.set_server_name(strId);
				reqMsg.set_server_cur_count(0);
				reqMsg.set_server_ip(ip);
				reqMsg.set_server_port(nPort);
				reqMsg.set_server_max_online(maxConnect);
				reqMsg.set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
				reqMsg.set_server_type(serverType);

				auto pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
				*(pServerData->pData) = reqMsg;
				mMasterMap.AddElement(serverID, pServerData);
			}
		}
	}

	return true;
}

void MasterNet_ServerModule::OnSocketEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet)
{
	//std::cout << "OnSocketEvent::thread id=" << GetCurrentThreadId() << std::endl;

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

void MasterNet_ServerModule::OnClientDisconnect(const SQUICK_SOCKET nAddress)
{
	
	SQUICK_SHARE_PTR<ServerData> pServerData = mWorldMap.First();
	while (pServerData)
	{
		if (nAddress == pServerData->nFD)
		{
			pServerData->pData->set_server_state(SquickStruct::ServerState::SERVER_CRASH);
			pServerData->nFD = 0;

			SynWorldToLoginAndWorld();
			return;
		}

		pServerData = mWorldMap.Next();
	}

	//////////////////////////////////////////////////////////////////////////

	int serverID = 0;
	pServerData = mLoginMap.First();
	while (pServerData)
	{
		if (nAddress == pServerData->nFD)
		{
			serverID = pServerData->pData->server_id();
			break;
		}

		pServerData = mLoginMap.Next();
	}

	mLoginMap.RemoveElement(serverID);

}

void MasterNet_ServerModule::OnClientConnected(const SQUICK_SOCKET nAddress)
{
}

void MasterNet_ServerModule::SynWorldToLoginAndWorld()
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mWorldMap.First();
	while (pServerData)
	{
		SquickStruct::ServerInfoReport* pData = xData.add_server_list();
		*pData = *(pServerData->pData);

		pServerData = mWorldMap.Next();
	}

	//loginserver
	pServerData = mLoginMap.First();
	while (pServerData)
	{
		m_pNetModule->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xData, pServerData->nFD);

		pServerData = mLoginMap.Next();
	}

	//world server
	pServerData = mWorldMap.First();
	while (pServerData)
	{
		SquickStruct::ServerInfoReportList xWorldData;
		const int nCurArea = m_pElementModule->GetPropertyInt(pServerData->pData->server_name(), excel::Server::Area());

		for (int i = 0; i < xData.server_list_size(); ++i)
		{
			const SquickStruct::ServerInfoReport& xServerInfo = xData.server_list(i);
			//it must be the same area			
			const int nAreaID = m_pElementModule->GetPropertyInt(xServerInfo.server_name(), excel::Server::Area());
			if (nAreaID == nCurArea)
			{
				SquickStruct::ServerInfoReport* pData = xWorldData.add_server_list();
				*pData = *(pServerData->pData);
			}
		}

		m_pNetModule->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xWorldData, pServerData->nFD);

		pServerData = mWorldMap.Next();
	}
	
}

void MasterNet_ServerModule::LogGameServer()
{
	if (mnLastLogTime + 60 > GetPluginManager()->GetNowTime())
	{
		return;
	}

	mnLastLogTime = GetPluginManager()->GetNowTime();

	//////////////////////////////////////////////////////////////////////////

	m_pLogModule->LogInfo(Guid(), "Begin Log WorldServer Info", "");

	SQUICK_SHARE_PTR<ServerData> pGameData = mWorldMap.First();
	while (pGameData)
	{
		std::ostringstream stream;
		stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id() << " State: " << SquickStruct::ServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip() << " FD: " << pGameData->nFD;
		m_pLogModule->LogInfo(Guid(), stream);

		pGameData = mWorldMap.Next();
	}

	m_pLogModule->LogInfo(Guid(), "End Log WorldServer Info", "");

	m_pLogModule->LogInfo(Guid(), "Begin Log LoginServer Info", "");

	//////////////////////////////////////////////////////////////////////////
	pGameData = mLoginMap.First();
	while (pGameData)
	{
		std::ostringstream stream;
		stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id() << " State: " << SquickStruct::ServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip() << " FD: " << pGameData->nFD;
		m_pLogModule->LogInfo(Guid(), stream);

		pGameData = mLoginMap.Next();
	}

	m_pLogModule->LogInfo(Guid(), "End Log LoginServer Info", "");

}

void MasterNet_ServerModule::OnHeartBeat(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
}

void MasterNet_ServerModule::InvalidMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	printf("Net || umsgID=%d\n", msgID);
}

void MasterNet_ServerModule::OnServerReport(const SQUICK_SOCKET nFd, const int msgId, const char* buffer, const uint32_t len)
{
	Guid xGUID;
	SquickStruct::ServerInfoReport msg;
	if (!m_pNetModule->ReceivePB(msgId, buffer, len, msg, xGUID))
	{
		return;
	}

	std::shared_ptr<ServerData> pServerData;
	switch (msg.server_type())
	{
		case SQUICK_SERVER_TYPES::SQUICK_ST_LOGIN:
		{
			pServerData = mLoginMap.GetElement(msg.server_id());
			if (!pServerData)
			{
				pServerData = std::shared_ptr<ServerData>(new ServerData());
				mLoginMap.AddElement(msg.server_id(), pServerData);
			}
		}
		break;
		case SQUICK_SERVER_TYPES::SQUICK_ST_WORLD:
		{
			pServerData = mWorldMap.GetElement(msg.server_id());
			if (!pServerData)
			{
				pServerData = std::shared_ptr<ServerData>(new ServerData());
				mWorldMap.AddElement(msg.server_id(), pServerData);
			}
		}
		break;
		case SQUICK_SERVER_TYPES::SQUICK_ST_PROXY:
		{
			pServerData = mProxyMap.GetElement(msg.server_id());
			if (!pServerData)
			{
				pServerData = std::shared_ptr<ServerData>(new ServerData());
				mProxyMap.AddElement(msg.server_id(), pServerData);
			}
		}
		break;
		case SQUICK_SERVER_TYPES::SQUICK_ST_GAME:
		{
			pServerData = mGameMap.GetElement(msg.server_id());
			if (!pServerData)
			{
				pServerData = std::shared_ptr<ServerData>(new ServerData());
				mGameMap.AddElement(msg.server_id(), pServerData);
			}
		}
		break;
		case SQUICK_SERVER_TYPES::SQUICK_ST_GATEWAY:
		{
			pServerData = mGatewayMap.GetElement(msg.server_id());
			if (!pServerData)
			{
				pServerData = std::shared_ptr<ServerData>(new ServerData());
				mGatewayMap.AddElement(msg.server_id(), pServerData);
			}
		}
		break;
		case SQUICK_SERVER_TYPES::SQUICK_ST_PVP_MANAGER:
		{
			pServerData = mPvpManagerMap.GetElement(msg.server_id());
			if (!pServerData)
			{
				pServerData = std::shared_ptr<ServerData>(new ServerData());
				mPvpManagerMap.AddElement(msg.server_id(), pServerData);
			}
		}
		break;

		default:
		{
			//m_pLogModule->LogError("UNKNOW SERVER TYPE", msg.server_type());
		}
		break;
	}

	//udate status
	if (pServerData)
	{
		pServerData->nFD = nFd;
		*(pServerData->pData) = msg;
	}
}

// 获取服务状态
std::string MasterNet_ServerModule::GetServersStatus()
{
	using json = nlohmann::json;
	json statusRoot;

	statusRoot["code"] = 0;
	statusRoot["errMsg"] = "";
	statusRoot["nowTime"] = pPluginManager->GetNowTime();
	
	std::shared_ptr<ServerData> pServerData = mMasterMap.First();
	int i = 0;
	while (pServerData)
	{
		json s;
		s["serverId"] = pServerData->pData->server_id();
		s["servrName"] = pServerData->pData->server_name().c_str();
		s["ip"] = pServerData->pData->server_ip().c_str();
		s["port"] = pServerData->pData->server_port();
		s["onlineCount"] = pServerData->pData->server_cur_count();
		s["status"] = (int)pServerData->pData->server_state();
		statusRoot["master" + std::to_string(i)] = s;
		i++;
		pServerData = mMasterMap.Next();
	}
	
	pServerData = mLoginMap.First();
	while (pServerData)
	{
		json s;
		s["serverId"] = pServerData->pData->server_id();
		s["servrName"] = pServerData->pData->server_name().c_str();
		s["ip"] = pServerData->pData->server_ip().c_str();
		s["port"] = pServerData->pData->server_port();
		s["onlineCount"] = pServerData->pData->server_cur_count();
		s["status"] = (int)pServerData->pData->server_state();
		statusRoot["login" + std::to_string(i)] = s;
		i++;
		pServerData = mMasterMap.Next();
	}
	
	pServerData = mWorldMap.First();
	while (pServerData.get())
	{
		json s;
		s["serverId"] = pServerData->pData->server_id();
		s["servrName"] = pServerData->pData->server_name().c_str();
		s["ip"] = pServerData->pData->server_ip().c_str();
		s["port"] = pServerData->pData->server_port();
		s["onlineCount"] = pServerData->pData->server_cur_count();
		s["status"] = (int)pServerData->pData->server_state();
		statusRoot["world" + std::to_string(i)] = s;
		i++;
		pServerData = mMasterMap.Next();
	}

	pServerData = mProxyMap.First();
	while (pServerData.get())
	{
		json s;
		s["serverId"] = pServerData->pData->server_id();
		s["servrName"] = pServerData->pData->server_name().c_str();
		s["ip"] = pServerData->pData->server_ip().c_str();
		s["port"] = pServerData->pData->server_port();
		s["onlineCount"] = pServerData->pData->server_cur_count();
		s["status"] = (int)pServerData->pData->server_state();
		statusRoot["proxy" + std::to_string(i)] = s;
		i++;
		pServerData = mMasterMap.Next();
	}

	pServerData = mGameMap.First();
	while (pServerData.get())
	{
		json s;
		s["serverId"] = pServerData->pData->server_id();
		s["servrName"] = pServerData->pData->server_name().c_str();
		s["ip"] = pServerData->pData->server_ip().c_str();
		s["port"] = pServerData->pData->server_port();
		s["onlineCount"] = pServerData->pData->server_cur_count();
		s["status"] = (int)pServerData->pData->server_state();
		statusRoot["game" + std::to_string(i)] = s;
		i++;
		pServerData = mMasterMap.Next();
	}

	pServerData = mGatewayMap.First();
	while (pServerData.get())
	{
		json s;
		s["serverId"] = pServerData->pData->server_id();
		s["servrName"] = pServerData->pData->server_name().c_str();
		s["ip"] = pServerData->pData->server_ip().c_str();
		s["port"] = pServerData->pData->server_port();
		s["onlineCount"] = pServerData->pData->server_cur_count();
		s["status"] = (int)pServerData->pData->server_state();
		statusRoot["gateway" + std::to_string(i)] = s;
		i++;
		pServerData = mMasterMap.Next();
	}

	pServerData = mPvpManagerMap.First();
	while (pServerData.get())
	{
		json s;
		s["serverId"] = pServerData->pData->server_id();
		s["servrName"] = pServerData->pData->server_name().c_str();
		s["ip"] = pServerData->pData->server_ip().c_str();
		s["port"] = pServerData->pData->server_port();
		s["onlineCount"] = pServerData->pData->server_cur_count();
		s["status"] = (int)pServerData->pData->server_state();
		statusRoot["pvp_manager" + std::to_string(i)] = s;
		i++;
		pServerData = mMasterMap.Next();
	}
	
	return statusRoot.dump();
}

