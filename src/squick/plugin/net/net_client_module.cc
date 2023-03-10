
#include "net_module.h"
#include "net_client_module.h"

NetClientModule::NetClientModule(IPluginManager* p)
{
    m_bIsUpdate = true;
    mnBufferSize = 0;
    pPluginManager = p;

	mnLastActionTime = GetPluginManager()->GetNowTime();
}

bool NetClientModule::Start()
{
	m_pLogModule = pPluginManager->FindModule<ILogModule>();

    for (int i = 0; i < SQUICK_SERVER_TYPES::SQUICK_ST_MAX; ++i)
    {
        INetClientModule::AddEventCallBack((SQUICK_SERVER_TYPES) i, this, &NetClientModule::OnSocketEvent);
    }

    return true;
}

bool NetClientModule::AfterStart()
{


    return true;
}

bool NetClientModule::BeforeDestory()
{
    return true;
}

bool NetClientModule::Destory()
{
    return true;
}

bool NetClientModule::Update()
{
    ProcessUpdate();
    ProcessAddNetConnect();

	if (mnLastActionTime + 10 > GetPluginManager()->GetNowTime())
	{
		return true;
	}

	mnLastActionTime = GetPluginManager()->GetNowTime();

	//LogServerInfo();

    return true;
}

void NetClientModule::RemoveReceiveCallBack(const SQUICK_SERVER_TYPES eType, const uint16_t msgID)
{
    SQUICK_SHARE_PTR<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (xCallBack)
    {
        std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator it = xCallBack->mxReceiveCallBack.find(msgID);
        if (xCallBack->mxReceiveCallBack.end() != it)
        {
            xCallBack->mxReceiveCallBack.erase(it);
        }
    }
}

void NetClientModule::AddServer(const ConnectData& xInfo)
{
    mxTempNetList.push_back(xInfo);
}

unsigned int NetClientModule::ExpandBufferSize(const unsigned int size)
{
    if (size > 0)
    {
        mnBufferSize = size;
    }
    return mnBufferSize;
}

int NetClientModule::AddReceiveCallBack(const SQUICK_SERVER_TYPES eType, const uint16_t msgID,
                                           NET_RECEIVE_FUNCTOR_PTR functorPtr)
{
    SQUICK_SHARE_PTR<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (!xCallBack)
    {
        xCallBack = SQUICK_SHARE_PTR<CallBack>(SQUICK_NEW CallBack);
        mxCallBack.AddElement(eType, xCallBack);
    }

	std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator itor = xCallBack->mxReceiveCallBack.find(msgID);
	if (itor == xCallBack->mxReceiveCallBack.end())
	{
		std::list<NET_RECEIVE_FUNCTOR_PTR> xList;
		xList.push_back(functorPtr);

    	xCallBack->mxReceiveCallBack.insert(std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::value_type(msgID, xList));
	}
	else
	{
		xCallBack->mxReceiveCallBack.at(msgID).push_back(functorPtr);
	}

    return 0;
}

int NetClientModule::AddReceiveCallBack(const SQUICK_SERVER_TYPES eType, NET_RECEIVE_FUNCTOR_PTR functorPtr)
{
    SQUICK_SHARE_PTR<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (!xCallBack)
    {
        xCallBack = SQUICK_SHARE_PTR<CallBack>(SQUICK_NEW CallBack);
        mxCallBack.AddElement(eType, xCallBack);
    }

    xCallBack->mxCallBackList.push_back(functorPtr);

    return 0;
}


int NetClientModule::AddEventCallBack(const SQUICK_SERVER_TYPES eType, NET_EVENT_FUNCTOR_PTR functorPtr)
{
    SQUICK_SHARE_PTR<CallBack> xCallBack = mxCallBack.GetElement(eType);
    if (!xCallBack)
    {
        xCallBack = SQUICK_SHARE_PTR<CallBack>(SQUICK_NEW CallBack);
        mxCallBack.AddElement(eType, xCallBack);
    }

    xCallBack->mxEventCallBack.push_back(functorPtr);
    return 0;
}

void NetClientModule::SendByServerIDWithOutHead(const int serverID, const uint16_t msgID, const std::string & strData)
{
	SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.GetElement(serverID);
	if (pServer)
	{
		SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
		if (pNetModule.get())
		{
			if (!pNetModule->SendMsgWithOutHead(msgID, strData, 0))
			{
				std::ostringstream stream;
				stream << " SendMsgWithOutHead failed " << serverID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
		}
	}
	else
	{
		std::ostringstream stream;
		stream << " can't find the server, target server id: " << serverID;
		stream << " msg id " << msgID;
		m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
	}
}

void NetClientModule::SendByServerID(const int serverID, const uint16_t msgID, const std::string& strData)
{
	SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.GetElement(serverID);
	if (pServer)
	{
		SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
		if (pNetModule.get())
		{
			if (!pNetModule->SendMsg(msgID, strData, 0))
			{
				std::ostringstream stream;
				stream << " SendMsgWithOutHead failed " << serverID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
		}
	}
	else
	{
		std::ostringstream stream;
		stream << " can't find the server " << serverID;
		stream << " msg id " << msgID;
		m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
	}
}

void NetClientModule::SendByServerID(const int serverID, const uint16_t msgID, const std::string & strData, const Guid id)
{
	SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.GetElement(serverID);
	if (pServer)
	{
		SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
		if (pNetModule.get())
		{
			if (!pNetModule->SendMsg(msgID, strData, 0, id))
			{
				std::ostringstream stream;
				stream << " SendMsgWithOutHead failed " << serverID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
		}
	}
	else
	{
		std::ostringstream stream;
		stream << " can't find the server " << serverID;
		stream << " msg id " << msgID;
		m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
	}
}

void NetClientModule::SendToAllServerWithOutHead(const uint16_t msgID, const std::string & strData)
{
	SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.First();
	while (pServer)
	{
		SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
		if (pNetModule)
		{
			if (!pNetModule->SendMsgWithOutHead(msgID, strData, 0))
			{
				std::ostringstream stream;
				stream << " SendMsgWithOutHead failed " << pServer->nGameID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
		}

		pServer = mxServerMap.Next();
	}
}

void NetClientModule::SendToAllServer(const uint16_t msgID, const std::string& strData)
{
    SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.First();
    while (pServer)
    {
        SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
        if (pNetModule)
        {
			if (!pNetModule->SendMsg(msgID, strData, 0))
			{
				std::ostringstream stream;
				stream << " SendMsgWithOutHead failed " << pServer->nGameID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
        }

        pServer = mxServerMap.Next();
    }
}

void NetClientModule::SendToAllServer(const uint16_t msgID, const std::string & strData, const Guid id)
{
	SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.First();
	while (pServer)
	{
		SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
		if (pNetModule)
		{
			if (!pNetModule->SendMsg(msgID, strData, 0, id))
			{
				std::ostringstream stream;
				stream << " SendMsgWithOutHead failed " << pServer->nGameID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
		}

		pServer = mxServerMap.Next();
	}
}

void NetClientModule::SendToAllServerWithOutHead(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const std::string & strData)
{
	SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.First();
	while (pServer)
	{
		SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
		if (pNetModule && eType == pServer->eServerType)
		{
			if (!pNetModule->SendMsgWithOutHead(msgID, strData, 0))
			{
				std::ostringstream stream;
				stream << " SendMsgWithOutHead failed " << pServer->nGameID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
		}

		pServer = mxServerMap.Next();
	}
}

void NetClientModule::SendToAllServer(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const std::string& strData)
{
    SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.First();
    while (pServer)
    {
        SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
        if (pNetModule && eType == pServer->eServerType)
        {
			if (!pNetModule->SendMsg(msgID, strData, 0))
			{
				std::ostringstream stream;
				stream << " SendMsgWithOutHead failed " << pServer->nGameID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
        }

        pServer = mxServerMap.Next();
    }
}

void NetClientModule::SendToAllServer(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const std::string & strData, const Guid id)
{
	SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.First();
	while (pServer)
	{
		SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
		if (pNetModule && eType == pServer->eServerType)
		{
			if (!pNetModule->SendMsg(msgID, strData, 0, id))
			{
				std::ostringstream stream;
				stream << " SendMsgWithOutHead failed " << pServer->nGameID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
		}

		pServer = mxServerMap.Next();
	}
}

void NetClientModule::SendToServerByPB(const int serverID, const uint16_t msgID, const google::protobuf::Message& xData)
{
    SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.GetElement(serverID);
    if (pServer)
    {
        SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
        if (pNetModule)
        {
			if (!pNetModule->SendMsgPB(msgID, xData, 0))
			{
				std::ostringstream stream;
				stream << " SendMsgPB failed " << pServer->nGameID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
		   }
        }
    }
	else
	{
		std::ostringstream stream;
		stream << " can't find the server " << serverID;
		stream << " msg id " << msgID;
		m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
	}
}

void NetClientModule::SendToServerByPB(const int serverID, const uint16_t msgID, const google::protobuf::Message & xData, const Guid id)
{
	SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.GetElement(serverID);
	if (pServer)
	{
		SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
		if (pNetModule)
		{
			if (!pNetModule->SendMsgPB(msgID, xData, 0, id))
			{
				std::ostringstream stream;
				stream << " SendMsgPB failed " << pServer->nGameID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
		}
	}
	else
	{
		std::ostringstream stream;
		stream << " can't find the server " << serverID;
		stream << " msg id " << msgID;
		m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
	}
}

void NetClientModule::SendToAllServerByPB(const uint16_t msgID, const google::protobuf::Message& xData, const Guid id)
{
    SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.First();
    while (pServer)
    {
        SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
        if (pNetModule)
        {
			if (!pNetModule->SendMsgPB(msgID, xData, 0, id))
			{
				std::ostringstream stream;
				stream << " SendMsgPB failed " << pServer->nGameID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
        }

        pServer = mxServerMap.Next();
    }
}

void NetClientModule::SendToAllServerByPB(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const google::protobuf::Message& xData, const Guid id)
{
    SQUICK_SHARE_PTR<ConnectData> pServer = mxServerMap.First();
    while (pServer)
    {
        SQUICK_SHARE_PTR<INetModule> pNetModule = pServer->mxNetModule;
        if (pNetModule && eType == pServer->eServerType && pServer->eState == ConnectDataState::NORMAL)
        {
            if (!pNetModule->SendMsgPB(msgID, xData, 0, id))
			{
				std::ostringstream stream;
				stream << " SendMsgPB failed " << pServer->nGameID;
				stream << " msg id " << msgID;
				m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
			}
        }

        pServer = mxServerMap.Next();
    }
}

void NetClientModule::SendBySuitWithOutHead(const SQUICK_SERVER_TYPES eType, const std::string & strHashKey, const uint16_t msgID, const std::string & strData)
{
	uint32_t nCRC32 = SquickProtocol::CRC32(strHashKey);
	SendBySuitWithOutHead(eType, nCRC32, msgID, strData);
}

void NetClientModule::SendBySuit(const SQUICK_SERVER_TYPES eType, const std::string& strHashKey, const uint16_t msgID,
                                    const std::string& strData)
{
    uint32_t nCRC32 = SquickProtocol::CRC32(strHashKey);
    SendBySuit(eType, nCRC32, msgID, strData);
}

void NetClientModule::SendBySuit(const SQUICK_SERVER_TYPES eType, const std::string & strHashKey, const uint16_t msgID, const std::string & strData, const Guid id)
{
	uint32_t nCRC32 = SquickProtocol::CRC32(strHashKey);
	SendBySuit(eType, nCRC32, msgID, strData, id);
}

void NetClientModule::SendBySuitWithOutHead(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const std::string & strData)
{
	SQUICK_SHARE_PTR<NFConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(eType);
	if (xConnectDataMap)
	{
		SQUICK_SHARE_PTR<ConnectData> pConnectData = xConnectDataMap->GetElementBySuit(nHashKey32);
		if (pConnectData)
		{
			SendByServerIDWithOutHead(pConnectData->nGameID, msgID, strData);
		}
	}
	else
	{
		std::ostringstream stream;
		stream << " can't find the server type " << eType;
		stream << " msg id " << msgID;
		m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
	}
}

void NetClientModule::SendBySuit(const SQUICK_SERVER_TYPES eType, const int nHashKey, const uint16_t msgID, const std::string& strData)
{
	SQUICK_SHARE_PTR<NFConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(eType);
	if (xConnectDataMap)
	{
		SQUICK_SHARE_PTR<ConnectData> pConnectData = xConnectDataMap->GetElementBySuit(nHashKey);
		if (pConnectData)
		{
			SendByServerID(pConnectData->nGameID, msgID, strData);
		}
	}
	else
	{
		std::ostringstream stream;
		stream << " can't find the server type " << eType;
		stream << " msg id " << msgID;
		m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
	}
}

void NetClientModule::SendBySuit(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const std::string & strData, const Guid id)
{
	SQUICK_SHARE_PTR<NFConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(eType);
	if (xConnectDataMap)
	{
		SQUICK_SHARE_PTR<ConnectData> pConnectData = xConnectDataMap->GetElementBySuit(nHashKey32);
		if (pConnectData)
		{
			SendByServerID(pConnectData->nGameID, msgID, strData, id);
		}
	}
	else
	{
		std::ostringstream stream;
		stream << " can't find the server type " << eType;
		stream << " msg id " << msgID;
		m_pLogModule->LogError(stream, __FUNCTION__, __LINE__);
	}
}

void NetClientModule::SendSuitByPB(const SQUICK_SERVER_TYPES eType, const std::string& strHashKey, const uint16_t msgID, const google::protobuf::Message& xData)
{
    uint32_t nCRC32 = SquickProtocol::CRC32(strHashKey);
    SendSuitByPB(eType, nCRC32, msgID, xData);
}

void NetClientModule::SendSuitByPB(const SQUICK_SERVER_TYPES eType, const std::string & strHashKey, const uint16_t msgID, const google::protobuf::Message & xData, const Guid id)
{
	uint32_t nCRC32 = SquickProtocol::CRC32(strHashKey);
	SendSuitByPB(eType, nCRC32, msgID, xData, id);
}

void NetClientModule::SendSuitByPB(const SQUICK_SERVER_TYPES eType, const int nHashKey, const uint16_t msgID, const google::protobuf::Message& xData)
{
    SQUICK_SHARE_PTR<NFConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(eType);
    if (xConnectDataMap)
    {
        SQUICK_SHARE_PTR<ConnectData> pConnectData = xConnectDataMap->GetElementBySuit(nHashKey);
        if (pConnectData)
        {
            SendToServerByPB(pConnectData->nGameID, msgID, xData);
        }
    }
}

void NetClientModule::SendSuitByPB(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const google::protobuf::Message & xData, const Guid id)
{
	SQUICK_SHARE_PTR<NFConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(eType);
	if (xConnectDataMap)
	{
		SQUICK_SHARE_PTR<ConnectData> pConnectData = xConnectDataMap->GetElementBySuit(nHashKey32);
		if (pConnectData)
		{
			SendToServerByPB(pConnectData->nGameID, msgID, xData, id);
		}
	}
}

SQUICK_SHARE_PTR<ConnectData> NetClientModule::GetServerNetInfo(const SQUICK_SERVER_TYPES eType)
{
    SQUICK_SHARE_PTR<NFConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(eType);
    if (xConnectDataMap)
    {
        return xConnectDataMap->GetElementBySuitRandom();
    }

    return nullptr;
}

SQUICK_SHARE_PTR<ConnectData> NetClientModule::GetServerNetInfo(const int serverID)
{
    return mxServerMap.GetElement(serverID);
}

MapEx<int, ConnectData>& NetClientModule::GetServerList()
{
    return mxServerMap;
}

SQUICK_SHARE_PTR<ConnectData> NetClientModule::GetServerNetInfo(const INet* pNet)
{
    int serverID = 0;
    for (SQUICK_SHARE_PTR<ConnectData> pData = mxServerMap.First(serverID);
         pData != NULL; pData = mxServerMap.Next(serverID))
    {
        if (pData->mxNetModule && pNet == pData->mxNetModule->GetNet())
        {
            return pData;
        }
    }

    return SQUICK_SHARE_PTR<ConnectData>(NULL);
}

void NetClientModule::StartCallBacks(SQUICK_SHARE_PTR<ConnectData> pServerData)
{
	std::ostringstream stream;
	stream << "AddServer Type: " << pServerData->eServerType << " Server ID: " << pServerData->nGameID << " State: "
		<< pServerData->eState << " IP: " << pServerData->ip << " Port: " << pServerData->nPort;

	m_pLogModule->LogInfo(stream.str());

    SQUICK_SHARE_PTR<CallBack> xCallBack = mxCallBack.GetElement(pServerData->eServerType);
    if (!xCallBack)
    {
        xCallBack = SQUICK_SHARE_PTR<CallBack>(SQUICK_NEW CallBack);
        mxCallBack.AddElement(pServerData->eServerType, xCallBack);
    }

    //add msg callback
    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>>::iterator itReciveCB = xCallBack->mxReceiveCallBack.begin();
    for (; xCallBack->mxReceiveCallBack.end() != itReciveCB; ++itReciveCB)
    {
		std::list<NET_RECEIVE_FUNCTOR_PTR>& xList = itReciveCB->second;
		for (std::list<NET_RECEIVE_FUNCTOR_PTR>::iterator itList = xList.begin(); itList != xList.end(); ++itList)
		{
			pServerData->mxNetModule->AddReceiveCallBack(itReciveCB->first, *itList);
		}
    }

    //add event callback
    std::list<NET_EVENT_FUNCTOR_PTR>::iterator itEventCB = xCallBack->mxEventCallBack.begin();
    for (; xCallBack->mxEventCallBack.end() != itEventCB; ++itEventCB)
    {
        pServerData->mxNetModule->AddEventCallBack(*itEventCB);
    }

    std::list<NET_RECEIVE_FUNCTOR_PTR>::iterator itCB = xCallBack->mxCallBackList.begin();
    for (; xCallBack->mxCallBackList.end() != itCB; ++itCB)
    {
        pServerData->mxNetModule->AddReceiveCallBack(*itCB);
    }
}

void NetClientModule::ProcessUpdate()
{
    SQUICK_SHARE_PTR<ConnectData> pServerData = mxServerMap.First();
    while (pServerData)
    {
        switch (pServerData->eState)
        {
            case ConnectDataState::DISCONNECT:
            {
                if (NULL != pServerData->mxNetModule)
                {
                    pServerData->mxNetModule = nullptr;
                    pServerData->eState = ConnectDataState::RECONNECT;
                }
            }
                break;
            case ConnectDataState::CONNECTING:
            {
                if (pServerData->mxNetModule)
                {
                    pServerData->mxNetModule->Update();
                }
            }
                break;
            case ConnectDataState::NORMAL:
            {
                if (pServerData->mxNetModule)
                {
                    pServerData->mxNetModule->Update();

                    KeepState(pServerData);
                }
            }
                break;
            case ConnectDataState::RECONNECT:
            {
                if ((pServerData->mnLastActionTime + 10) >= GetPluginManager()->GetNowTime())
                {
                    break;
                }

                if (nullptr != pServerData->mxNetModule)
                {
                    pServerData->mxNetModule = nullptr;
                }

                pServerData->eState = ConnectDataState::CONNECTING;
                pServerData->mxNetModule = SQUICK_SHARE_PTR<INetModule>(SQUICK_NEW NetModule(pPluginManager));

				pServerData->mxNetModule->Awake();
				pServerData->mxNetModule->Start();
				pServerData->mxNetModule->AfterStart();
				pServerData->mxNetModule->ReadyUpdate();

                pServerData->mxNetModule->Startialization(pServerData->ip.c_str(), pServerData->nPort);

                StartCallBacks(pServerData);
            }
                break;
            default:
                break;
        }

        pServerData = mxServerMap.Next();
    }
}

// 打印服务状态
void NetClientModule::LogServerInfo()
{
	bool error = false;
	std::ostringstream stream;
	//stream << "This is a client, begin to print Server Info-------------------" << std::endl;

    ConnectData* pServerData = mxServerMap.FirstNude();
    while (nullptr != pServerData)
    {
        stream << "\nNetClientModule::LogServerInfo:\nServer Info: Type: " << typeid(pServerData->eServerType).name() << " Server ID: " << pServerData->nGameID << " State: " <<  pServerData->eState << " IP: " << pServerData->ip << " Port: " << pServerData->nPort << std::endl;

		if (pServerData->eState != ConnectDataState::NORMAL)
		{
			error = true;
		}

        pServerData = mxServerMap.NextNude();
    }

	//stream << "\nThis is a client, end to print Server Info--------------------- " << std::endl;

    if (error)
	{
		//stream << " in " << SQUICK_DEBUG_INFO << std::endl;
		m_pLogModule->LogError(stream.str());
	}
	else
	{
		m_pLogModule->LogInfo(stream.str());
	}
}

void NetClientModule::KeepState(SQUICK_SHARE_PTR<ConnectData> pServerData)
{
    if (pServerData->mnLastActionTime + 10 > GetPluginManager()->GetNowTime())
    {
        return;
    }

    pServerData->mnLastActionTime = GetPluginManager()->GetNowTime();

	//send message
}

void NetClientModule::OnSocketEvent(const SQUICK_SOCKET fd, const SQUICK_NET_EVENT eEvent, INet* pNet)
{
    if (eEvent & SQUICK_NET_EVENT::SQUICK_NET_EVENT_CONNECTED)
    {
        OnConnected(fd, pNet);
    }
	else
    {
        OnDisConnected(fd, pNet);
    }
}

int NetClientModule::OnConnected(const SQUICK_SOCKET fd, INet* pNet)
{
    SQUICK_SHARE_PTR<ConnectData> pServerInfo = GetServerNetInfo(pNet);
    if (pServerInfo)
    {
        /////////////////////////////////////////////////////////////////////////////////////
        //AddServerWeightData(pServerInfo);
        pServerInfo->eState = ConnectDataState::NORMAL;

        //for type--suit
        SQUICK_SHARE_PTR<NFConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(pServerInfo->eServerType);
        if (!xConnectDataMap)
        {
            xConnectDataMap = SQUICK_SHARE_PTR<NFConsistentHashMapEx<int, ConnectData>>(SQUICK_NEW NFConsistentHashMapEx<int, ConnectData>());
            mxServerTypeMap.AddElement(pServerInfo->eServerType, xConnectDataMap);
        }

        xConnectDataMap->AddElement(pServerInfo->nGameID, pServerInfo);
    }

    return 0;
}

int NetClientModule::OnDisConnected(const SQUICK_SOCKET fd, INet* pNet)
{
    SQUICK_SHARE_PTR<ConnectData> pServerInfo = GetServerNetInfo(pNet);
    if (nullptr != pServerInfo)
    {
        /////////////////////////////////////////////////////////////////////////////////////
        //RemoveServerWeightData(pServerInfo);
        pServerInfo->eState = ConnectDataState::DISCONNECT;
        pServerInfo->mnLastActionTime = GetPluginManager()->GetNowTime();

        //for type--suit
        SQUICK_SHARE_PTR<NFConsistentHashMapEx<int, ConnectData>> xConnectDataMap = mxServerTypeMap.GetElement(pServerInfo->eServerType);
        if (xConnectDataMap)
        {
			xConnectDataMap->RemoveElement(pServerInfo->nGameID);
        }
    }

    return 0;
}

void NetClientModule::ProcessAddNetConnect()
{
    std::list<ConnectData>::iterator it = mxTempNetList.begin();
    for (; it != mxTempNetList.end(); ++it)
    {
        const ConnectData& xInfo = *it;
        SQUICK_SHARE_PTR<ConnectData> xServerData = mxServerMap.GetElement(xInfo.nGameID);
        if (nullptr == xServerData)
        {
            xServerData = SQUICK_SHARE_PTR<ConnectData>(SQUICK_NEW ConnectData());

            xServerData->nGameID = xInfo.nGameID;
            xServerData->eServerType = xInfo.eServerType;
            xServerData->ip = xInfo.ip;
            xServerData->name = xInfo.name;
            xServerData->eState = ConnectDataState::CONNECTING;
            xServerData->nPort = xInfo.nPort;
            xServerData->mnLastActionTime = GetPluginManager()->GetNowTime();

            xServerData->mxNetModule = SQUICK_SHARE_PTR<INetModule>(SQUICK_NEW NetModule(pPluginManager));

			xServerData->mxNetModule->Awake();
			xServerData->mxNetModule->Start();
			xServerData->mxNetModule->AfterStart();
			xServerData->mxNetModule->ReadyUpdate();

            xServerData->mxNetModule->Startialization(xServerData->ip.c_str(), xServerData->nPort);
            xServerData->mxNetModule->ExpandBufferSize((unsigned int)mnBufferSize);

            StartCallBacks(xServerData);

            mxServerMap.AddElement(xInfo.nGameID, xServerData);
        }
		else
		{
			xServerData->nWorkLoad = xInfo.nWorkLoad;
		}
    }

    mxTempNetList.clear();
}