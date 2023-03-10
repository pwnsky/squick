
#include "server_module.h"
#include "plugin.h"
#include <squick/struct/struct.h>

bool WorldNet_ServerModule::Start()
{
	this->pPluginManager->SetAppType(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD);

	m_pNetModule = pPluginManager->FindModule<INetModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pClassModule = pPluginManager->FindModule<IClassModule>();
	m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
	m_pThreadPoolModule = pPluginManager->FindModule<IThreadPoolModule>();
	
    return true;
}

bool WorldNet_ServerModule::AfterStart()
{
	m_pNetModule->AddReceiveCallBack(SquickStruct::PROXY_TO_WORLD_REGISTERED, this, &WorldNet_ServerModule::OnProxyServerRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::PROXY_TO_WORLD_UNREGISTERED, this, &WorldNet_ServerModule::OnProxyServerUnRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::PROXY_TO_WORLD_REFRESH, this, &WorldNet_ServerModule::OnRefreshProxyServerInfoProcess);

	m_pNetModule->AddReceiveCallBack(SquickStruct::GAME_TO_WORLD_REGISTERED, this, &WorldNet_ServerModule::OnGameServerRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::GAME_TO_WORLD_UNREGISTERED, this, &WorldNet_ServerModule::OnGameServerUnRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::GAME_TO_WORLD_REFRESH, this, &WorldNet_ServerModule::OnRefreshGameServerInfoProcess);
	
	m_pNetModule->AddReceiveCallBack(SquickStruct::DB_TO_WORLD_REGISTERED, this, &WorldNet_ServerModule::OnDBServerRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::DB_TO_WORLD_UNREGISTERED, this, &WorldNet_ServerModule::OnDBServerUnRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::DB_TO_WORLD_REFRESH, this, &WorldNet_ServerModule::OnRefreshDBServerInfoProcess);

	m_pNetModule->AddReceiveCallBack(SquickStruct::GAMEPLAY_MANAGER_TO_WORLD_REGISTERED, this, &WorldNet_ServerModule::OnGameplayManagerServerRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::GAMEPLAY_MANAGER_TO_WORLD_UNREGISTERED, this, &WorldNet_ServerModule::OnGameplayManagerServerUnRegisteredProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::GAMEPLAY_MANAGER_TO_WORLD_REFRESH, this, &WorldNet_ServerModule::OnRefreshGameplayManagerServerInfoProcess);

	m_pNetModule->AddReceiveCallBack(SquickStruct::STS_SERVER_REPORT, this, &WorldNet_ServerModule::OnTransmitServerReport);

    m_pNetModule->AddEventCallBack(this, &WorldNet_ServerModule::OnSocketEvent);
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
            if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_WORLD && pPluginManager->GetAppID() == serverID)
            {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
				const int nCpus = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());

				mAreaID = m_pElementModule->GetPropertyInt32(strId, excel::Server::Area());
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

void WorldNet_ServerModule::OnServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();

		const int nCurAppID = pPluginManager->GetAppID();
		std::vector<std::string>::const_iterator itr =
			std::find_if(strIdList.begin(), strIdList.end(), [&](const std::string& strConfigId)
		{
			return nCurAppID == m_pElementModule->GetPropertyInt32(strConfigId, excel::Server::ServerID());
		});

		if (strIdList.end() == itr)
		{
			std::ostringstream strLog;
			strLog << "Cannot find current server, AppID = " << nCurAppID;
			m_pLogModule->LogError(NULL_OBJECT, strLog, __FILE__, __LINE__);

			return;
		}

		const int nCurArea = m_pElementModule->GetPropertyInt32(*itr, excel::Server::Area());

		Guid nPlayerID;
		SquickStruct::ServerInfoReportList xMsg;
		if (!INetModule::ReceivePB(msgID, msg, len, xMsg, nPlayerID))
		{
			return;
		}

		mWorldMap.ClearAll();

		for (int i = 0; i < xMsg.server_list_size(); ++i)
		{
			const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
			const int nAreaID = m_pElementModule->GetPropertyInt(xData.server_name(), excel::Server::Area());
			if (xData.server_type() == SQUICK_SERVER_TYPES::SQUICK_ST_WORLD
				&& nCurArea == nAreaID)
			{
				SQUICK_SHARE_PTR<ServerData> pServerData = mWorldMap.GetElement(xData.server_id());
				if (!pServerData)
				{
					pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
					mWorldMap.AddElement(xData.server_id(), pServerData);
				}

				pServerData->nFD = sockIndex;
				*(pServerData->pData) = xData;

				m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
			}
		}
	}

	//sync to proxy
	SynWorldToProxy();

	//sync to game
	SynWorldToGame();

	// 同步 World 到 Pvp Manager
	SynWorldToGameplayManager();

}

bool WorldNet_ServerModule::Destory()
{

    return true;
}

bool WorldNet_ServerModule::Update()
{

	if (mnLastCheckTime + 10 > GetPluginManager()->GetNowTime())
	{
		return true;
	}
	// 定时同步服务器表

	SynDBToGame();
	SynGameToProxy();
	SynWorldToProxy();
	SynWorldToGame();
	SynWorldToDB();

	SynGameToGameplayManager();
	SynWorldToGameplayManager();

    //LogGameServer();

	return true;
}

void WorldNet_ServerModule::OnGameServerRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();

		const int nCurAppID = pPluginManager->GetAppID();
		std::vector<std::string>::const_iterator itr =
			std::find_if(strIdList.begin(), strIdList.end(), [&](const std::string& strConfigId)
		{
			return nCurAppID == m_pElementModule->GetPropertyInt32(strConfigId, excel::Server::ServerID());
		});

		if (strIdList.end() == itr)
		{
			std::ostringstream strLog;
			strLog << "Cannot find current server, AppID = " << nCurAppID;
			m_pLogModule->LogError(NULL_OBJECT, strLog, __FILE__, __LINE__);

			return;
		}

		const int nCurArea = m_pElementModule->GetPropertyInt32(*itr, excel::Server::Area());

		Guid nPlayerID;
		SquickStruct::ServerInfoReportList xMsg;
		if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, nPlayerID))
		{
			return;
		}

		for (int i = 0; i < xMsg.server_list_size(); ++i)
		{
			const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
			const int nAreaID = m_pElementModule->GetPropertyInt(xData.server_name(), excel::Server::Area());
			if (nAreaID == nCurArea)
			{
				SQUICK_SHARE_PTR<ServerData> pServerData = mGameMap.GetElement(xData.server_id());
				if (!pServerData)
				{

					pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
					mGameMap.AddElement(xData.server_id(), pServerData);
				}

				pServerData->nFD = sockIndex;
				*(pServerData->pData) = xData;

				m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
			}
			else
			{
				m_pLogModule->LogError(Guid(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
			}
		}
	}

    SynGameToProxy();
}

void WorldNet_ServerModule::OnGameServerUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
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
        mGameMap.RemoveElement(xData.server_id());

        m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
    }
}

void WorldNet_ServerModule::OnRefreshGameServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
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

        SQUICK_SHARE_PTR<ServerData> pServerData =  mGameMap.GetElement(xData.server_id());
        if (!pServerData)
        {
            pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
            mGameMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->nFD = sockIndex;
        *(pServerData->pData) = xData;

        m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
    }

    SynGameToProxy();
}

void WorldNet_ServerModule::OnProxyServerRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();

		const int nCurAppID = pPluginManager->GetAppID();
		std::vector<std::string>::const_iterator itr =
			std::find_if(strIdList.begin(), strIdList.end(), [&](const std::string& strConfigId)
		{
			return nCurAppID == m_pElementModule->GetPropertyInt32(strConfigId, excel::Server::ServerID());
		});

		if (strIdList.end() == itr)
		{
			std::ostringstream strLog;
			strLog << "Cannot find current server, AppID = " << nCurAppID;
			m_pLogModule->LogError(NULL_OBJECT, strLog, __FILE__, __LINE__);

			return;
		}

		const int nCurArea = m_pElementModule->GetPropertyInt32(*itr, excel::Server::Area());

		Guid nPlayerID;
		SquickStruct::ServerInfoReportList xMsg;
		if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, nPlayerID))
		{
			return;
		}

		for (int i = 0; i < xMsg.server_list_size(); ++i)
		{
			const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
			const int nAreaID = m_pElementModule->GetPropertyInt(xData.server_name(), excel::Server::Area());
			if (nAreaID == nCurArea) // 同一区服的就同步转发表
			{
				SQUICK_SHARE_PTR<ServerData> pServerData = mProxyMap.GetElement(xData.server_id());
				if (!pServerData)
				{
					pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
					mProxyMap.AddElement(xData.server_id(), pServerData);
				}

				pServerData->nFD = sockIndex;
				*(pServerData->pData) = xData;

				m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");

				SynGameToProxy(sockIndex);
			}
			else
			{
				m_pLogModule->LogError(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");
			}
		}
	}
}

void WorldNet_ServerModule::OnProxyServerUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
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

        mGameMap.RemoveElement(xData.server_id());

        m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy UnRegistered");
    }
}

void WorldNet_ServerModule::OnRefreshProxyServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
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

        SQUICK_SHARE_PTR<ServerData> pServerData =  mProxyMap.GetElement(xData.server_id());
        if (!pServerData)
        {
            pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
            mProxyMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->nFD = sockIndex;
        *(pServerData->pData) = xData;

        m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");

        SynGameToProxy(sockIndex);
    }
}

void WorldNet_ServerModule::OnDBServerRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();

		const int nCurAppID = pPluginManager->GetAppID();
		std::vector<std::string>::const_iterator itr =
			std::find_if(strIdList.begin(), strIdList.end(), [&](const std::string& strConfigId)
		{
			return nCurAppID == m_pElementModule->GetPropertyInt32(strConfigId, excel::Server::ServerID());
		});

		if (strIdList.end() == itr)
		{
			std::ostringstream strLog;
			strLog << "Cannot find current server, AppID = " << nCurAppID;
			m_pLogModule->LogError(NULL_OBJECT, strLog, __FILE__, __LINE__);

			return;
		}

		const int nCurArea = m_pElementModule->GetPropertyInt32(*itr, excel::Server::Area());

		Guid nPlayerID;
		SquickStruct::ServerInfoReportList xMsg;
		if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, nPlayerID))
		{
			return;
		}

		for (int i = 0; i < xMsg.server_list_size(); ++i)
		{
			const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
			const int nAreaID = m_pElementModule->GetPropertyInt(xData.server_name(), excel::Server::Area());
			if (nAreaID == nCurArea)
			{

				SQUICK_SHARE_PTR<ServerData> pServerData = mDBMap.GetElement(xData.server_id());
				if (!pServerData)
				{
					pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
					mDBMap.AddElement(xData.server_id(), pServerData);
				}

				pServerData->nFD = sockIndex;
				*(pServerData->pData) = xData;

				m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "DBServerRegistered");
			}
			else
			{
				m_pLogModule->LogError(Guid(0, xData.server_id()), xData.server_name(), "DBServerRegistered");
			}
		}

		SynDBToGame();
	}
}

void WorldNet_ServerModule::OnDBServerUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
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

		mDBMap.RemoveElement(xData.server_id());

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy UnRegistered");
	}
}

void WorldNet_ServerModule::OnRefreshDBServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
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

		SQUICK_SHARE_PTR<ServerData> pServerData = mDBMap.GetElement(xData.server_id());
		if (!pServerData)
		{
			pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
			mDBMap.AddElement(xData.server_id(), pServerData);
		}

		pServerData->nFD = sockIndex;
		*(pServerData->pData) = xData;

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");

		SynDBToGame(sockIndex);
		// SynDBToPvp()
	}
}

// PVP  Server

void WorldNet_ServerModule::OnGameplayManagerServerRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();

		const int nCurAppID = pPluginManager->GetAppID();
		std::vector<std::string>::const_iterator itr =
			std::find_if(strIdList.begin(), strIdList.end(), [&](const std::string& strConfigId)
				{
					return nCurAppID == m_pElementModule->GetPropertyInt32(strConfigId, excel::Server::ServerID());
				});

		if (strIdList.end() == itr)
		{
			std::ostringstream strLog;
			strLog << "Cannot find current server, AppID = " << nCurAppID;
			m_pLogModule->LogError(NULL_OBJECT, strLog, __FILE__, __LINE__);

			return;
		}

		const int nCurArea = m_pElementModule->GetPropertyInt32(*itr, excel::Server::Area());

		Guid nPlayerID;
		SquickStruct::ServerInfoReportList xMsg;
		if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, nPlayerID))
		{
			return;
		}

		for (int i = 0; i < xMsg.server_list_size(); ++i)
		{
			const SquickStruct::ServerInfoReport& xData = xMsg.server_list(i);
			const int nAreaID = m_pElementModule->GetPropertyInt(xData.server_name(), excel::Server::Area());
			if (nAreaID == nCurArea) // 同一区服的就同步转发表
			{
				SQUICK_SHARE_PTR<ServerData> pServerData = mGameplayManagerMap.GetElement(xData.server_id());
				if (!pServerData)
				{
					pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
					mGameplayManagerMap.AddElement(xData.server_id(), pServerData);
				}

				pServerData->nFD = sockIndex;
				*(pServerData->pData) = xData;

				m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");

				SynGameToProxy(sockIndex);
			}
			else
			{
				m_pLogModule->LogError(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");
			}
		}
	}
}

void WorldNet_ServerModule::OnGameplayManagerServerUnRegisteredProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
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

		mGameMap.RemoveElement(xData.server_id());

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy UnRegistered");
	}
}

void WorldNet_ServerModule::OnRefreshGameplayManagerServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
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

		SQUICK_SHARE_PTR<ServerData> pServerData = mGameplayManagerMap.GetElement(xData.server_id());
		if (!pServerData)
		{
			pServerData = SQUICK_SHARE_PTR<ServerData>(SQUICK_NEW ServerData());
			mGameplayManagerMap.AddElement(xData.server_id(), pServerData);
		}

		pServerData->nFD = sockIndex;
		*(pServerData->pData) = xData;

		m_pLogModule->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");

		SynGameToProxy(sockIndex);
	}
}
// -------------------



void WorldNet_ServerModule::OnSocketEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet)
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


// 同步转发表
void WorldNet_ServerModule::SynGameToProxy()
{
    SquickStruct::ServerInfoReportList xData;

    SQUICK_SHARE_PTR<ServerData> pServerData =  mProxyMap.First();
    while (pServerData)
    {
        SynGameToProxy(pServerData->nFD);

        pServerData = mProxyMap.Next();
    }
}

void WorldNet_ServerModule::SynGameToProxy(const SQUICK_SOCKET nFD)
{
    SquickStruct::ServerInfoReportList xData;

    SQUICK_SHARE_PTR<ServerData> pServerData =  mGameMap.First();
    while (pServerData)
    {
        SquickStruct::ServerInfoReport* pData = xData.add_server_list();
        *pData = *(pServerData->pData);

        pServerData = mGameMap.Next();
    }

	m_pNetModule->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xData, nFD);
}

void WorldNet_ServerModule::SynWorldToProxy()
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mProxyMap.First();
	while (pServerData)
	{
		SynWorldToProxy(pServerData->nFD);

		pServerData = mProxyMap.Next();
	}
}

void WorldNet_ServerModule::SynWorldToProxy(const SQUICK_SOCKET nFD)
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mWorldMap.First();
	while (pServerData)
	{
		SquickStruct::ServerInfoReport* pData = xData.add_server_list();
		*pData = *(pServerData->pData);

		pServerData = mWorldMap.Next();
	}

	m_pNetModule->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xData, nFD);
}

// Start: PVP Manager <-> Game < - > World 服务器之间的转发表同步


void WorldNet_ServerModule::SynGameToGameplayManager()
{
	SquickStruct::ServerInfoReportList xData;
	// 依次同步到 Pvp Manager 服务器上
	SQUICK_SHARE_PTR<ServerData> pServerData = mGameplayManagerMap.First();
	while (pServerData)
	{
		SynGameToGameplayManager(pServerData->nFD);

		pServerData = mGameplayManagerMap.Next();
	}
}

void WorldNet_ServerModule::SynGameToGameplayManager(const SQUICK_SOCKET nFD)
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mGameMap.First();
	while (pServerData)
	{
		SquickStruct::ServerInfoReport* pData = xData.add_server_list();
		*pData = *(pServerData->pData);
		pServerData = mGameMap.Next();
	}

	m_pNetModule->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xData, nFD);
}

void WorldNet_ServerModule::SynWorldToGameplayManager()
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mGameplayManagerMap.First();
	while (pServerData)
	{
		SynWorldToGameplayManager(pServerData->nFD);

		pServerData = mGameplayManagerMap.Next();
	}
}

void WorldNet_ServerModule::SynWorldToGameplayManager(const SQUICK_SOCKET nFD)
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mWorldMap.First();
	while (pServerData)
	{
		SquickStruct::ServerInfoReport* pData = xData.add_server_list();
		*pData = *(pServerData->pData);

		pServerData = mWorldMap.Next();
	}

	m_pNetModule->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xData, nFD);
}

// End: PVP Manager <-> Game < - > World 




void WorldNet_ServerModule::SynWorldToGame()
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mGameMap.First();
	while (pServerData)
	{
		if (pServerData->pData->server_state() != SquickStruct::ServerState::SERVER_MAINTEN
			&& pServerData->pData->server_state() != SquickStruct::ServerState::SERVER_CRASH)
		{
			SynWorldToGame(pServerData->nFD);
		}

		pServerData = mGameMap.Next();
	}
}

void WorldNet_ServerModule::SynWorldToGame(const SQUICK_SOCKET nFD)
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mWorldMap.First();
	while (pServerData)
	{
		SquickStruct::ServerInfoReport* pData = xData.add_server_list();
		*pData = *(pServerData->pData);

		pServerData = mWorldMap.Next();
	}

	m_pNetModule->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xData, nFD);
}


void WorldNet_ServerModule::SynWorldToDB()
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mDBMap.First();
	while (pServerData)
	{
		if (pServerData->pData->server_state() != SquickStruct::ServerState::SERVER_MAINTEN
			&& pServerData->pData->server_state() != SquickStruct::ServerState::SERVER_CRASH)
		{
			SynWorldToDB(pServerData->nFD);
		}

		pServerData = mDBMap.Next();
	}
}

void WorldNet_ServerModule::SynWorldToDB(const SQUICK_SOCKET nFD)
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mWorldMap.First();
	while (pServerData)
	{

		SquickStruct::ServerInfoReport* pData = xData.add_server_list();
		*pData = *(pServerData->pData);

		pServerData = mWorldMap.Next();
	}

	m_pNetModule->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xData, nFD);
}

void WorldNet_ServerModule::SynDBToGame()
{
	SquickStruct::ServerInfoReportList xData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mGameMap.First();
	while (pServerData)
	{
		if (pServerData->pData->server_state() != SquickStruct::ServerState::SERVER_MAINTEN
			&& pServerData->pData->server_state() != SquickStruct::ServerState::SERVER_CRASH)
		{
			SynDBToGame(pServerData->nFD);
		}

		pServerData = mGameMap.Next();
	}
}

void WorldNet_ServerModule::SynDBToGame(const SQUICK_SOCKET nFD)
{
	SquickStruct::ServerInfoReportList xData;
	
	SQUICK_SHARE_PTR<ServerData> pServerData = mDBMap.First();
	while (pServerData)
	{
		SquickStruct::ServerInfoReport* pData = xData.add_server_list();
		*pData = *(pServerData->pData);
		
		pServerData = mDBMap.Next();
	}
	
	m_pNetModule->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xData, nFD);
}

void WorldNet_ServerModule::OnClientDisconnect(const SQUICK_SOCKET nAddress)
{
    SQUICK_SHARE_PTR<ServerData> pServerData =  mGameMap.First();
    while (pServerData)
    {
        if (nAddress == pServerData->nFD)
        {
            pServerData->pData->set_server_state(SquickStruct::ServerState::SERVER_CRASH);
            pServerData->nFD = 0;

			ServerReport(pServerData->pData->server_id(), SquickStruct::ServerState::SERVER_CRASH);
            SynGameToProxy();
            break;
        }

        pServerData = mGameMap.Next();
    }

    //////////////////////////////////////////////////////////////////////////

    pServerData =  mProxyMap.First();
    while (pServerData)
    {
        if (nAddress == pServerData->nFD)
        {
            pServerData->pData->set_server_state(SquickStruct::ServerState::SERVER_CRASH);
            pServerData->nFD = 0;

            int serverID = pServerData->pData->server_id();
            mProxyMap.RemoveElement(serverID);

			ServerReport(pServerData->pData->server_id(), SquickStruct::ServerState::SERVER_CRASH);
			SynGameToProxy();
            break;
        }

        pServerData = mProxyMap.Next();
    }


	//////////////////////////////////////////////////////////////////////////

	pServerData = mDBMap.First();
	while (pServerData)
	{
		if (nAddress == pServerData->nFD)
		{
            pServerData->pData->set_server_state(SquickStruct::ServerState::SERVER_CRASH);
            pServerData->nFD = 0;

			int serverID = pServerData->pData->server_id();
			mDBMap.RemoveElement(serverID);

			ServerReport(pServerData->pData->server_id(), SquickStruct::ServerState::SERVER_CRASH);
			SynDBToGame();
			break;
		}

		pServerData = mDBMap.Next();
	}
}

void WorldNet_ServerModule::OnClientConnected(const SQUICK_SOCKET nAddress)
{

}

void WorldNet_ServerModule::LogGameServer()
{
    mnLastCheckTime = GetPluginManager()->GetNowTime();

    m_pLogModule->LogInfo(Guid(), "--------------------Begin Log GameServer Info", "");

    SQUICK_SHARE_PTR<ServerData> pGameData = mGameMap.First();
    while (pGameData)
    {
        std::ostringstream stream;
        stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id() << " State: " <<  SquickStruct::ServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip() << " FD: " << pGameData->nFD;

        m_pLogModule->LogInfo(Guid(), stream);

        pGameData = mGameMap.Next();
    }

    m_pLogModule->LogInfo(Guid(), "--------------------End Log GameServer Info", "");

    m_pLogModule->LogInfo(Guid(), "--------------------Begin Log ProxyServer Info", "");

    pGameData = mProxyMap.First();
    while (pGameData)
    {
        std::ostringstream stream;
        stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id() << " State: " <<  SquickStruct::ServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip() << " FD: " << pGameData->nFD;

        m_pLogModule->LogInfo(Guid(), stream);

        pGameData = mProxyMap.Next();
    }

    m_pLogModule->LogInfo(Guid(), "--------------------End Log ProxyServer Info", "");


	m_pLogModule->LogInfo(Guid(), "--------------------Begin Log DBServer Info", "");

	pGameData = mDBMap.First();
	while (pGameData)
	{
		std::ostringstream stream;
		stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id() << " State: " << SquickStruct::ServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip() << " FD: " << pGameData->nFD;

		m_pLogModule->LogInfo(Guid(), stream);

		pGameData = mDBMap.Next();
	}

	m_pLogModule->LogInfo(Guid(), "--------------------End Log DBServer Info", "");
}


void WorldNet_ServerModule::OnOnlineProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	/*
	std::cout << "ONline::::::\n";
    CLIENT_MSG_PROCESS_NO_OBJECT(msgID, msg, len, SquickStruct::RoleOnlineNotify);

    Guid selfId = INetModule::ProtobufToStruct(xMsg.self());

    SQUICK_SHARE_PTR<PlayerData> playerData = mPlayersData.GetElement(selfId);
    if (playerData)
    {
		playerData->name = xMsg.name();
		playerData->bp = xMsg.bp();

    }
	else
	{
		playerData = SQUICK_SHARE_PTR<PlayerData>(SQUICK_NEW PlayerData(selfId));

		playerData->name = xMsg.name();
		playerData->bp = xMsg.bp();

		mPlayersData.AddElement(selfId, playerData);
	}

	playerData->OnLine(xMsg.game(), xMsg.proxy());

	for (int i = 0; i < mPlayerOnLineCallBackFunc.size(); ++i)
	{
		auto callback = mPlayerOnLineCallBackFunc[i];
		callback->operator()(selfId);
	} */
}

void WorldNet_ServerModule::OnOfflineProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	/*
	std::cout << "Offline::::::\n";
    CLIENT_MSG_PROCESS_NO_OBJECT(msgID, msg, len, SquickStruct::RoleOfflineNotify);
    Guid self = INetModule::ProtobufToStruct(xMsg.self());


	SQUICK_SHARE_PTR<PlayerData> playerData = mPlayersData.GetElement(self);
	if (playerData)
	{
		for (int i = 0; i < mPlayerOffLineCallBackFunc.size(); ++i)
		{
			auto callback = mPlayerOffLineCallBackFunc[i];
			callback->operator()(self);
		}


		playerData->OffLine();
	} */
}

void WorldNet_ServerModule::OnTransmitServerReport(const SQUICK_SOCKET nFd, const int msgId, const char *buffer, const uint32_t len)
{
    Guid xGUID;
	SquickStruct::ServerInfoReport msg;
	if (!m_pNetModule->ReceivePB(msgId, buffer, len, msg, xGUID))
	{
		return;
	}


	m_pNetClientModule->SendToAllServerByPB(SQUICK_SERVER_TYPES::SQUICK_ST_MASTER, SquickStruct::STS_SERVER_REPORT, msg, Guid());

}

bool WorldNet_ServerModule::SendMsgToGame(const int gameID, const int msgID, const std::string &xData)
{
	SQUICK_SHARE_PTR<ServerData> pData = mGameMap.GetElement(gameID);
	if (pData)
	{
		const SQUICK_SOCKET nFD = pData->nFD;
		m_pNetModule->SendMsg(msgID, xData, nFD, Guid());

		return true;
	}

	return false;
}

bool WorldNet_ServerModule::SendMsgToGame(const int gameID, const int msgID, const google::protobuf::Message &xData)
{
	SQUICK_SHARE_PTR<ServerData> pData = mGameMap.GetElement(gameID);
	if (pData)
	{
		const SQUICK_SOCKET nFD = pData->nFD;
		m_pNetModule->SendMsgPB(msgID, xData, nFD, Guid());

		return true;
	}

	return false;
}

bool WorldNet_ServerModule::SendMsgToGamePlayer(const Guid nPlayer, const int msgID, const std::string& xData)
{
    SQUICK_SHARE_PTR<PlayerData> playerData = mPlayersData.GetElement(nPlayer);
    if (playerData)
    {
        SQUICK_SHARE_PTR<ServerData> pData = mGameMap.GetElement(playerData->gameID);
        if (pData)
        {
            const SQUICK_SOCKET nFD = pData->nFD;
            m_pNetModule->SendMsg(msgID, xData, nFD, nPlayer);

            return true;
        }
    }

    return false;
}

bool WorldNet_ServerModule::SendMsgToGamePlayer(const Guid nPlayer, const int msgID, const google::protobuf::Message& xData)
{
	SQUICK_SHARE_PTR<PlayerData> playerData = mPlayersData.GetElement(nPlayer);
	if (playerData)
	{
		SQUICK_SHARE_PTR<ServerData> pData = mGameMap.GetElement(playerData->gameID);
		if (pData)
		{
			const SQUICK_SOCKET nFD = pData->nFD;
			m_pNetModule->SendMsgPB(msgID, xData, nFD, nPlayer);

			return true;
		}
	}

	return false;
}

bool WorldNet_ServerModule::SendMsgToGamePlayer(const DataList& argObjectVar, const int msgID, google::protobuf::Message& xData)
{
    for (int i = 0; i < argObjectVar.GetCount(); i++)
    {
        const Guid& nPlayer = argObjectVar.Object(i);
		SendMsgToGamePlayer(nPlayer, msgID, xData);
    }

    return true;
}

SQUICK_SHARE_PTR<ServerData> WorldNet_ServerModule::GetSuitProxyToEnter()
{
	int nConnectNum = 99999;
	SQUICK_SHARE_PTR<ServerData> pReturnServerData;

	SQUICK_SHARE_PTR<ServerData> pServerData = mProxyMap.First();
	while (pServerData)
	{
		if (pServerData->pData->server_cur_count() < nConnectNum)
		{
			nConnectNum = pServerData->pData->server_cur_count();
			pReturnServerData = pServerData;
		}

		pServerData = mProxyMap.Next();
	}

	return pReturnServerData;
}

SQUICK_SHARE_PTR<ServerData> WorldNet_ServerModule::GetSuitGameToEnter(const int arg)
{
	return mGameMap.GetElementBySuit(arg);
}

int WorldNet_ServerModule::GetPlayerGameID(const Guid self)
{
	SQUICK_SHARE_PTR<PlayerData> playerData = mPlayersData.GetElement(self);
	if (playerData)
	{
		return playerData->gameID;
	}

	return 0;
}


const std::vector<Guid>& WorldNet_ServerModule::GetOnlinePlayers()
{
	static std::vector<Guid> players;
	players.clear();

	SQUICK_SHARE_PTR<PlayerData> playerData = mPlayersData.First();
	while (playerData)
	{
		players.push_back(playerData->self);

		playerData = mPlayersData.Next();
	}

    return players;
}

SQUICK_SHARE_PTR<IWorldNet_ServerModule::PlayerData> WorldNet_ServerModule::GetPlayerData(const Guid& id)
{
	//如果没有，就从redis load

	return mPlayersData.GetElement(id);
}

void WorldNet_ServerModule::ServerReport(int reportServerId, SquickStruct::ServerState serverStatus)
{
	std::shared_ptr<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
		for (int i = 0; i < strIdList.size(); ++i)
		{
			const std::string& strId = strIdList[i];

			const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
			const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
			if (reportServerId == serverID)
			{
				const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
				const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
				const std::string& name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
				const std::string& ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

				SquickStruct::ServerInfoReport reqMsg;

				reqMsg.set_server_id(serverID);
				reqMsg.set_server_name(strId);
				reqMsg.set_server_cur_count(0);
				reqMsg.set_server_ip(ip);
				reqMsg.set_server_port(nPort);
				reqMsg.set_server_max_online(maxConnect);
				reqMsg.set_server_state(serverStatus);
				reqMsg.set_server_type(serverType);


				m_pNetClientModule->SendToAllServerByPB(SQUICK_SERVER_TYPES::SQUICK_ST_MASTER, SquickStruct::STS_SERVER_REPORT, reqMsg, Guid());
			}
		}
	}
}

bool WorldNet_ServerModule::AddOnLineReceiveCallBack(std::shared_ptr<std::function<void(const Guid)>> cb)
{
	mPlayerOnLineCallBackFunc.push_back(cb);
	return true;
}

bool WorldNet_ServerModule::AddOffLineReceiveCallBack(std::shared_ptr<std::function<void(const Guid)>> cb)
{
	mPlayerOffLineCallBackFunc.push_back(cb);
	return true;
}

bool WorldNet_ServerModule::IsPrimaryWorldServer()
{
	auto serverData = mWorldMap.GetElementBySuitConsistent();
	if (serverData && serverData->pData->server_id() == GetPluginManager()->GetAppID())
	{
		return true;
	}

	return false;
}

int WorldNet_ServerModule::GetWorldAreaID()
{
	return mAreaID;
}
