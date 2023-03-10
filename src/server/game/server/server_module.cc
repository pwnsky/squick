#include "server_module.h"
#include <squick/plugin/kernel/i_event_module.h>
#include <squick/plugin/kernel/scene_module.h>
#include <forward_list>

namespace game::server {
bool GameServerNet_ServerModule::Start()
{
	this->pPluginManager->SetAppType(SQUICK_SERVER_TYPES::SQUICK_ST_GAME);

	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pClassModule = pPluginManager->FindModule<IClassModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();
	m_pEventModule = pPluginManager->FindModule<IEventModule>();
	m_pSceneModule = pPluginManager->FindModule<ISceneModule>();
	m_pScheduleModule = pPluginManager->FindModule<IScheduleModule>();
	
	m_pNetModule = pPluginManager->FindModule<INetModule>();
	m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
	m_pThreadPoolModule = pPluginManager->FindModule<IThreadPoolModule>();

	return true;
}

bool GameServerNet_ServerModule::AfterStart()
{

	m_pNetModule->AddReceiveCallBack(SquickStruct::PROXY_TO_GAME_REFRESH, this, &GameServerNet_ServerModule::OnRefreshProxyServerInfoProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::PROXY_TO_GAME_REGISTERED, this, &GameServerNet_ServerModule::OnProxyServerRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::PROXY_TO_GAME_UNREGISTERED, this, &GameServerNet_ServerModule::OnProxyServerUnRegisteredProcess);
	
	m_pNetModule->AddReceiveCallBack(SquickStruct::GAMEPLAY_MANAGER_TO_GAME_REFRESH, this, &GameServerNet_ServerModule::OnRefreshPvpManagerServerInfoProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::GAMEPLAY_MANAGER_TO_GAME_REGISTERED, this, &GameServerNet_ServerModule::OnPvpManagerServerRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::GAMEPLAY_MANAGER_TO_GAME_UNREGISTERED, this, &GameServerNet_ServerModule::OnPvpManagerServerUnRegisteredProcess);
	


	m_pNetModule->AddEventCallBack(this, &GameServerNet_ServerModule::OnSocketPSEvent);

	/////////////////////////////////////////////////////////////////////////

	SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
		for (int i = 0; i < strIdList.size(); ++i)
		{
			const std::string& strId = strIdList[i];

			const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
			const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
			if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_GAME && pPluginManager->GetAppID() == serverID)
			{
				const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
				const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
				const int nCpus = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());
				//const std::string& name = m_pElementModule->GetPropertyString(strId, SquickProtocol::Server::ID());
				//const std::string& ip = m_pElementModule->GetPropertyString(strId, SquickProtocol::Server::IP());
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

bool GameServerNet_ServerModule::Destory()
{

	return true;
}

bool GameServerNet_ServerModule::Update()
{
	return true;
}

void GameServerNet_ServerModule::OnSocketPSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet)
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

void GameServerNet_ServerModule::OnClientDisconnect(const SQUICK_SOCKET nAddress)
{
	int serverID = 0;
	SQUICK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.First();
	while (pServerData)
	{
		if (nAddress == pServerData->xServerData.nFD)
		{
			serverID = pServerData->xServerData.pData->server_id();
			break;
		}

		pServerData = mProxyMap.Next();
	}
	mProxyMap.RemoveElement(serverID);
}

void GameServerNet_ServerModule::OnClientConnected(const SQUICK_SOCKET nAddress)
{

}

void GameServerNet_ServerModule::OnProxyServerRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ServerInfoReportList xMsg;
	if (!INetModule::ReceivePB( msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
		SQUICK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.GetElement(xData.server_id());
		if (!pServerData)
		{
			pServerData = SQUICK_SHARE_PTR<GateServerInfo>(SQUICK_NEW GateServerInfo());
			mProxyMap.AddElement(xData.server_id(), pServerData);
		}

		pServerData->xServerData.nFD = sockIndex;
		*(pServerData->xServerData.pData) = xData;

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");
	}

	return;
}

void GameServerNet_ServerModule::OnProxyServerUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
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
		mProxyMap.RemoveElement(xData.server_id());


		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy UnRegistered");
	}

	return;
}

void GameServerNet_ServerModule::OnRefreshProxyServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
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
		SQUICK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.GetElement(xData.server_id());
		if (!pServerData)
		{
			pServerData = SQUICK_SHARE_PTR<GateServerInfo>(SQUICK_NEW GateServerInfo());
			mProxyMap.AddElement(xData.server_id(), pServerData);
		}

		pServerData->xServerData.nFD = sockIndex;
		*(pServerData->xServerData.pData) = xData;

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");
	}

	return;
}



void GameServerNet_ServerModule::OnPvpManagerServerRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ServerInfoReportList xMsg;
	if (!INetModule::ReceivePB(msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
		SQUICK_SHARE_PTR<GateServerInfo> pServerData = mGameplayManagerMap.GetElement(xData.server_id());
		if (!pServerData)
		{
			pServerData = SQUICK_SHARE_PTR<GateServerInfo>(SQUICK_NEW GateServerInfo());
			mGameplayManagerMap.AddElement(xData.server_id(), pServerData);
		}

		pServerData->xServerData.nFD = sockIndex;
		*(pServerData->xServerData.pData) = xData;

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Gameplay Manager Registered");
	}

	return;
}

void GameServerNet_ServerModule::OnPvpManagerServerUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ServerInfoReportList xMsg;
	if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
		mGameplayManagerMap.RemoveElement(xData.server_id());

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Gameplay Manager Registered");
	}

	return;
}

void GameServerNet_ServerModule::OnRefreshPvpManagerServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	SquickStruct::ServerInfoReportList xMsg;
	if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, nPlayerID))
	{
		return;
	}

	for (int i = 0; i < xMsg.server_list_size(); ++i)
	{
		const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
		SQUICK_SHARE_PTR<GateServerInfo> pServerData = mGameplayManagerMap.GetElement(xData.server_id());
		if (!pServerData)
		{
			pServerData = SQUICK_SHARE_PTR<GateServerInfo>(SQUICK_NEW GateServerInfo());
			mGameplayManagerMap.AddElement(xData.server_id(), pServerData);
		}

		pServerData->xServerData.nFD = sockIndex;
		*(pServerData->xServerData.pData) = xData;

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Gameplay Manager Registered");
	}

	return;
}



void GameServerNet_ServerModule::SendMsgPBToGate(const uint16_t msgID, google::protobuf::Message& xMsg, const Guid& self)
{
	SQUICK_SHARE_PTR<GateBaseInfo> pData = mRoleBaseData.GetElement(self);
	if (pData)
	{
		SQUICK_SHARE_PTR<GateServerInfo> pProxyData = mProxyMap.GetElement(pData->gateID);
		if (pProxyData)
		{
			m_pNetModule->SendMsgPB(msgID, xMsg, pProxyData->xServerData.nFD, pData->xClientID);
		}
	}
}

void GameServerNet_ServerModule::SendMsgToGate(const uint16_t msgID, const std::string& msg, const Guid& self)
{
	SQUICK_SHARE_PTR<GateBaseInfo> pData = mRoleBaseData.GetElement(self);
	if (pData)
	{
		SQUICK_SHARE_PTR<GateServerInfo> pProxyData = mProxyMap.GetElement(pData->gateID);
		if (pProxyData)
		{
			m_pNetModule->SendMsg(msgID, msg, pProxyData->xServerData.nFD, pData->xClientID);
		}
	}
}

// 发送给 Gameplay Manager 服务器
void GameServerNet_ServerModule::SendMsgPBToGameplayManager(const uint16_t msgID, google::protobuf::Message& xMsg)
{
	// 选择Gameplay转发表中的第一个Gameplay Manager进行发送,之后根据workload进行分配查找Gameplay Manager服务器
	GateServerInfo* pGameData = mGameplayManagerMap.FirstNude();
	if (pGameData) {
		dout << "发送给 Gameplay Manager 服务器: " << pGameData->xServerData.pData << std::endl;
		m_pNetModule->SendMsgPB(msgID, xMsg, pGameData->xServerData.nFD);
	}
	else {
		dout << "未找到 Gameplay Manager 服务器\n";
	}
}

void GameServerNet_ServerModule::SendMsgToGameplayManager(const uint16_t msgID, const std::string& msg)
{
	// 选择Gameplay转发表中的第一个Gameplay Manager进行发送
	GateServerInfo* pGameData = mGameplayManagerMap.FirstNude();
	if (pGameData) {
		m_pNetModule->SendMsg(msgID, msg, pGameData->xServerData.nFD);
	}
	else {
		dout << "未找到 Gameplay Manager 服务器\n";
	}
}

// 发送给Gameplay服务器
void GameServerNet_ServerModule::SendMsgPBToGameplay(const uint16_t msgID, google::protobuf::Message& xMsg, const Guid& self)
{
	GateServerInfo* pGameData = mGameplayManagerMap.FirstNude();
	if (pGameData) {
		dout << "发送给 Gameplay Manager 服务器: " << pGameData->xServerData.pData << std::endl;
		m_pNetModule->SendMsgPB(msgID, xMsg, pGameData->xServerData.nFD, self);
	}
	else {
		dout << "未找到 Gameplay Manager 服务器\n";
	}
}

void GameServerNet_ServerModule::SendMsgToGameplay(const uint16_t msgID, const std::string& msg, const Guid& self)
{
	GateServerInfo* pGameData = mGameplayManagerMap.FirstNude();
	if (pGameData) {
		dout << "发送给 Gameplay Manager 服务器: " << pGameData->xServerData.pData << std::endl;
		m_pNetModule->SendMsg(msgID, msg, pGameData->xServerData.nFD, self);
	}
	else {
		dout << "未找到 Gameplay Manager 服务器\n";
	}
}
// ---


void GameServerNet_ServerModule::SendGroupMsgPBToGate(const uint16_t msgID, google::protobuf::Message & xMsg, const int sceneID, const int groupID)
{
	//care: batch
	DataList xList;
	if (m_pKernelModule->GetGroupObjectList(sceneID, groupID, xList, true))
	{
		for (int i = 0; i < xList.GetCount(); ++i)
		{
			Guid xObject = xList.Object(i);
			this->SendMsgPBToGate(msgID, xMsg, xObject);
		}
	}
}

void GameServerNet_ServerModule::SendGroupMsgPBToGate(const uint16_t msgID, google::protobuf::Message & xMsg, const int sceneID, const int groupID, const Guid exceptID)
{
	DataList xList;
	if (m_pKernelModule->GetGroupObjectList(sceneID, groupID, xList, true))
	{
		for (int i = 0; i < xList.GetCount(); ++i)
		{
			Guid xObject = xList.Object(i);
			if (xObject != exceptID)
			{
				this->SendMsgPBToGate(msgID, xMsg, xObject);
			}
		}
	}
}

void GameServerNet_ServerModule::SendGroupMsgPBToGate(const uint16_t msgID, const std::string & msg, const int sceneID, const int groupID)
{
	//care: batch
	DataList xList;
	if (m_pKernelModule->GetGroupObjectList(sceneID, groupID, xList, true))
	{
		for (int i = 0; i < xList.GetCount(); ++i)
		{
			Guid xObject = xList.Object(i);
			this->SendMsgToGate(msgID, msg, xObject);
		}
	}
}

void GameServerNet_ServerModule::SendGroupMsgPBToGate(const uint16_t msgID, const std::string & msg, const int sceneID, const int groupID, const Guid exceptID)
{
	DataList xList;
	if (m_pKernelModule->GetGroupObjectList(sceneID, groupID, xList, true))
	{
		for (int i = 0; i < xList.GetCount(); ++i)
		{
			Guid xObject = xList.Object(i);
			if (xObject != exceptID)
			{
				this->SendMsgToGate(msgID, msg, xObject);
			}
		}
	}
}

bool GameServerNet_ServerModule::AddPlayerGateInfo(const Guid& roleID, const Guid& clientID, const int gateID)
{
    if (gateID <= 0)
    {
        return false;
    }

    if (clientID.IsNull())
    {
        return false;
    }

    SQUICK_SHARE_PTR<GameServerNet_ServerModule::GateBaseInfo> pBaseData = mRoleBaseData.GetElement(roleID);
    if (nullptr != pBaseData)
    {
        m_pLogModule->LogError(clientID, "player is exist, cannot enter game", __FUNCTION__, __LINE__);
        return false;
    }

    SQUICK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.GetElement(gateID);
    if (nullptr == pServerData)
    {
        return false;
    }

    if (!pServerData->xRoleInfo.insert(std::make_pair(roleID, pServerData->xServerData.nFD)).second)
    {
        return false;
    }

    if (!mRoleBaseData.AddElement(roleID, SQUICK_SHARE_PTR<GateBaseInfo>(SQUICK_NEW GateBaseInfo(gateID, clientID))))
    {
        pServerData->xRoleInfo.erase(roleID) ;
        return false;
    }

    return true;
}

/*
bool GameServerNet_ServerModule::AddPvpGateInfo(const Guid& clientID, const int gateID)
{
	if (gateID <= 0) {
		return false;
	}

	if (clientID.IsNull()) {
		return false;
	}

	SQUICK_SHARE_PTR<GateServerInfo> pServerData = mPvpManagerMap.GetElement(gateID);
	if (nullptr == pServerData) {
		return false;
	}

	if (!pServerData->xRoleInfo.insert(std::make_pair(clientID, pServerData->xServerData.nFD)).second) {
		return false;
	}

	return true;
}*/

bool GameServerNet_ServerModule::RemovePlayerGateInfo(const Guid& roleID)
{
    SQUICK_SHARE_PTR<GateBaseInfo> pBaseData = mRoleBaseData.GetElement(roleID);
    if (nullptr == pBaseData)
    {
        return false;
    }

    mRoleBaseData.RemoveElement(roleID);

    SQUICK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.GetElement(pBaseData->gateID);
    if (nullptr == pServerData)
    {
        return false;
    }

    pServerData->xRoleInfo.erase(roleID);
    return true;
}

SQUICK_SHARE_PTR<IGameServerNet_ServerModule::GateBaseInfo> GameServerNet_ServerModule::GetPlayerGateInfo(const Guid& roleID)
{
    return mRoleBaseData.GetElement(roleID);
}

SQUICK_SHARE_PTR<IGameServerNet_ServerModule::GateServerInfo> GameServerNet_ServerModule::GetGateServerInfo(const int gateID)
{
    return mProxyMap.GetElement(gateID);
}

SQUICK_SHARE_PTR<IGameServerNet_ServerModule::GateServerInfo> GameServerNet_ServerModule::GetGateServerInfoBySockIndex(const SQUICK_SOCKET sockIndex)
{
    int gateID = -1;
    SQUICK_SHARE_PTR<GateServerInfo> pServerData = mProxyMap.First();
    while (pServerData)
    {
        if (sockIndex == pServerData->xServerData.nFD)
        {
            gateID = pServerData->xServerData.pData->server_id();
            break;
        }

        pServerData = mProxyMap.Next();
    }

    if (gateID == -1)
    {
        return nullptr;
    }

    return pServerData;
}

void GameServerNet_ServerModule::OnTransWorld(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	std::string msgData;
	Guid nPlayer;
	int64_t nHasKey = 0;
	if (INetModule::ReceivePB( msgID, msg, len, msgData, nPlayer))
	{
		nHasKey = nPlayer.nData64;
	}

	m_pNetClientModule->SendBySuitWithOutHead(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, nHasKey, msgID, std::string(msg, len));
}
}