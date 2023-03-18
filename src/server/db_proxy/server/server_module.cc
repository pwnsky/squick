#include <squick/struct/struct.h>

#include "server_module.h"
#include "../logic/player_redis_module.h"
bool DBNet_ServerModule::Awake()
{
	this->pPluginManager->SetAppType(SQUICK_SERVER_TYPES::SQUICK_ST_DB);

	return true;
}

bool DBNet_ServerModule::Start()
{
	m_pNetModule = pPluginManager->FindModule<INetModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pClassModule = pPluginManager->FindModule<IClassModule>();
	m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
	m_pAccountRedisModule = pPluginManager->FindModule<IAccountRedisModule>();
	m_pPlayerRedisModule = pPluginManager->FindModule<IPlayerRedisModule>();
	m_pThreadPoolModule = pPluginManager->FindModule<IThreadPoolModule>();
	
    return true;
}

bool DBNet_ServerModule::AfterStart()
{
    m_pNetModule->AddEventCallBack(this, &DBNet_ServerModule::OnSocketEvent);
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
            if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_DB && pPluginManager->GetAppID() == serverID)
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

	m_pNetModule->AddReceiveCallBack(SquickStruct::DbProxyRPC::REQ_PLAYER_LIST, this, &DBNet_ServerModule::OnRequireRoleListProcess);
	
	m_pNetModule->AddReceiveCallBack(SquickStruct::DbProxyRPC::REQ_PLAYER_CREATE, this, &DBNet_ServerModule::OnCreateRoleGameProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::DbProxyRPC::REQ_PLAYER_DELETE, this, &DBNet_ServerModule::OnDeleteRoleGameProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::DbProxyRPC::REQ_PLAYER_DATA_LOAD, this, &DBNet_ServerModule::OnLoadRoleDataProcess);
	m_pNetModule->AddReceiveCallBack(SquickStruct::DbProxyRPC::REQ_PLAYER_DATA_SAVE, this, &DBNet_ServerModule::OnSaveRoleDataProcess);

    return true;
}

bool DBNet_ServerModule::Destory()
{

    return true;
}

bool DBNet_ServerModule::Update()
{
	return true;
}

// Socket 事件 
void DBNet_ServerModule::OnSocketEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet)
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

void DBNet_ServerModule::OnClientDisconnect(const SQUICK_SOCKET nAddress)
{
}

void DBNet_ServerModule::OnClientConnected(const SQUICK_SOCKET nAddress)
{
}

// 获取角色列表
void DBNet_ServerModule::OnRequireRoleListProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	/*
	Guid clientID;
	SquickStruct::ReqRoleList xMsg;
	if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID))
	{
		return;
	}

	Guid xPlayerID;
	std::string strRoleName;
	if (!m_pPlayerRedisModule->GetRoleInfo(xMsg.account(), strRoleName, xPlayerID))
	{
		SquickStruct::AckRoleLiteInfoList xAckRoleLiteInfoList;
		xAckRoleLiteInfoList.set_account(xMsg.account());
		m_pNetModule->SendMsgPB(SquickStruct::ACK_ROLE_LIST, xAckRoleLiteInfoList, sockIndex, clientID);
		return;
	}

	SquickStruct::AckRoleLiteInfoList xAckRoleLiteInfoList;
	xAckRoleLiteInfoList.set_account(xMsg.account());

	SquickStruct::RoleLiteInfo* pData = xAckRoleLiteInfoList.add_char_data();
	pData->mutable_id()->CopyFrom(INetModule::StructToProtobuf(xPlayerID));
	pData->set_game_id(pPluginManager->GetAppID());
	pData->set_last_offline_time(0);
	pData->set_last_offline_ip(0);
	pData->set_view_record("");

	m_pNetModule->SendMsgPB(SquickStruct::ACK_ROLE_LIST, xAckRoleLiteInfoList, sockIndex, clientID);
	*/
}

void DBNet_ServerModule::OnCreateRoleGameProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	/*
	Guid clientID;
	SquickStruct::ReqCreateRole xMsg;
	if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID))
	{
		return;
	}
	
	const std::string& account = xMsg.account();
	const std::string& name = xMsg.name();
	const int nHomeSceneID = 1;
#ifdef SQUICK_DEV
	std::cout << "DBNet_ServerModule::OnCreateRoleGameProcess account: " << account << std::endl;
#endif
	Guid xID = m_pKernelModule->CreateGUID();

	if (m_pPlayerRedisModule->CreateRole(account, name, xID, nHomeSceneID))
	{
		SquickStruct::AckRoleLiteInfoList xAckRoleLiteInfoList;
		xAckRoleLiteInfoList.set_account(account);

		SquickStruct::RoleLiteInfo* pData = xAckRoleLiteInfoList.add_char_data();
		pData->mutable_id()->CopyFrom(INetModule::StructToProtobuf(xID));
		pData->set_game_id(pPluginManager->GetAppID());
		pData->set_last_offline_time(0);
		pData->set_last_offline_ip(0);
		pData->set_view_record("");

#ifdef SQUICK_DEV
		std::cout << "创建角色成功 " << std::endl;
		std::cout << "响应给客户端 SquickStruct::ACK_ROLE_LIST : clienId" << std::endl;
#endif
		// 响应创建角色
		m_pNetModule->SendMsgPB(SquickStruct::ACK_ROLE_LIST, xAckRoleLiteInfoList, sockIndex, clientID);
	}
	*/
}

void DBNet_ServerModule::OnDeleteRoleGameProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	/*
	Guid clientID;
	SquickStruct::ReqDeleteRole xMsg;
	if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID))
	{
		return;
	}

	SquickStruct::AckRoleLiteInfoList xAckRoleLiteInfoList;
	xAckRoleLiteInfoList.set_account(xMsg.account());

	m_pNetModule->SendMsgPB(SquickStruct::ACK_ROLE_LIST, xAckRoleLiteInfoList, sockIndex, clientID);

	*/
}

void DBNet_ServerModule::OnLoadRoleDataProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	Guid clientID;
	SquickStruct::ReqEnter xMsg;
	if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID))
	{
		return;
	}

	dout << clientID.ToString() << "请求加载角色数据\n";

	//Guid roleID = INetModule::ProtobufToStruct(xMsg.object());
	Guid xID = m_pKernelModule->CreateGUID();
	SquickStruct::PlayerData xPlayerData;
	xPlayerData.mutable_object()->CopyFrom(INetModule::StructToProtobuf(xID));

	//PlayerRedisModule* pPlayerRedisModule = (PlayerRedisModule*)m_pPlayerRedisModule;
	//pPlayerRedisModule->LoadPlayerData(roleID, xPlayerData);

	m_pNetModule->SendMsgPB(SquickStruct::DbProxyRPC::ACK_PLAYER_DATA_LOAD, xPlayerData, sockIndex, clientID);
}

void DBNet_ServerModule::OnSaveRoleDataProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char * msg, const uint32_t len)
{
	/*
	Guid clientID;
	SquickStruct::RoleDataPack xMsg;
	if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, clientID))
	{
		return;
	}

	Guid roleID = INetModule::ProtobufToStruct(xMsg.id());

	PlayerRedisModule* pPlayerRedisModule = (PlayerRedisModule*)m_pPlayerRedisModule;
	pPlayerRedisModule->SavePlayerData(roleID, xMsg);

	*/
}
