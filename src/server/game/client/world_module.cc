

#include <squick/struct/struct.h>
#include <squick/plugin/net/i_net_module.h>

#include "plugin.h"
#include "world_module.h"

#include <server/db/logic/common_redis_module.h>

bool GameServerToWorldModule::Start()
{
	m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pClassModule = pPluginManager->FindModule<IClassModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();
	m_pGameServerNet_ServerModule = pPluginManager->FindModule<IGameServerNet_ServerModule>();

	return true;
}

bool GameServerToWorldModule::Destory()
{

	return true;
}


bool GameServerToWorldModule::Update()
{
	ServerReport();
	return true;
}

void GameServerToWorldModule::Register(INet* pNet)
{
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
				//const int nCpus = m_pElementModule->GetPropertyInt32(strId, SquickProtocol::Server::CpuCount());
				const std::string& name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
				const std::string& ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

				SquickStruct::ServerInfoReportList xMsg;
				SquickStruct::ServerInfoReport* pData = xMsg.add_server_list();

				pData->set_server_id(serverID);
				pData->set_server_name(strId);
				pData->set_server_cur_count(0);
				pData->set_server_ip(ip);
				pData->set_server_port(nPort);
				pData->set_server_max_online(maxConnect);
				pData->set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
				pData->set_server_type(serverType);

				SQUICK_SHARE_PTR<ConnectData> pServerData = m_pNetClientModule->GetServerNetInfo(pNet);
				if (pServerData)
				{
					int nTargetID = pServerData->nGameID;
					m_pNetClientModule->SendToServerByPB(nTargetID, SquickStruct::ServerRPC::GAME_TO_WORLD_REGISTERED, xMsg);

					m_pLogModule->LogInfo(Guid(0, pData->server_id()), pData->server_name(), "Register");
				}
			}
		}
	}
}

void GameServerToWorldModule::ServerReport()
{
	if (mLastReportTime + 10 > pPluginManager->GetNowTime())
	{
		return;
	}
	mLastReportTime = pPluginManager->GetNowTime();

	std::shared_ptr<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
	if (xLogicClass)
	{
		const std::vector<std::string>& strIdList = xLogicClass->GetIDList();
		for (int i = 0; i < strIdList.size(); ++i)
		{
			const std::string& strId = strIdList[i];

			const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
			const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
			if (pPluginManager->GetAppID() == serverID)
			{
				const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
				const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
				const std::string& name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
				const std::string& ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

				SquickStruct::ServerInfoReport reqMsg;

				reqMsg.set_server_id(serverID);
				reqMsg.set_server_name(strId);
				reqMsg.set_server_cur_count(m_pKernelModule->GetOnLineCount());
				reqMsg.set_server_ip(ip);
				reqMsg.set_server_port(nPort);
				reqMsg.set_server_max_online(maxConnect);
				reqMsg.set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
				reqMsg.set_server_type(serverType);

				m_pNetClientModule->SendToAllServerByPB(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::STS_SERVER_REPORT, reqMsg, Guid());
			}
		}
	}
}

bool GameServerToWorldModule::AfterStart()
{
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_INT, this, &GameServerToWorldModule::OnWorldPropertyIntProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_FLOAT, this, &GameServerToWorldModule::OnWorldPropertyFloatProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_STRING, this, &GameServerToWorldModule::OnWorldPropertyStringProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_OBJECT, this, &GameServerToWorldModule::OnWorldPropertyObjectProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_VECTOR2, this, &GameServerToWorldModule::OnWorldPropertyVector2Process);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_VECTOR3, this, &GameServerToWorldModule::OnWorldPropertyVector3Process);

	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_OBJECT_RECORD_ENTRY, this, &GameServerToWorldModule::OnWorldRecordEnterProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_ADD_ROW, this, &GameServerToWorldModule::OnWorldAddRowProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_REMOVE_ROW, this, &GameServerToWorldModule::OnWorldRemoveRowProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_SWAP_ROW, this, &GameServerToWorldModule::OnWorldSwapRowProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_INT, this, &GameServerToWorldModule::OnWorldRecordIntProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_FLOAT, this, &GameServerToWorldModule::OnWorldRecordFloatProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_STRING, this, &GameServerToWorldModule::OnWorldRecordStringProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_OBJECT, this, &GameServerToWorldModule::OnWorldRecordObjectProcess);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_VECTOR2, this, &GameServerToWorldModule::OnWorldRecordVector2Process);
	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_VECTOR3, this, &GameServerToWorldModule::OnWorldRecordVector3Process);

	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::STS_NET_INFO, this, &GameServerToWorldModule::OnServerInfoProcess);

	m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, this, &GameServerToWorldModule::TransPBToProxy);
	
	m_pNetClientModule->AddEventCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, this, &GameServerToWorldModule::OnSocketWSEvent);
	
	m_pKernelModule->AddClassCallBack(excel::Player::ThisName(), this, &GameServerToWorldModule::OnObjectClassEvent);

	m_pNetClientModule->ExpandBufferSize();

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
			NFASSERT(-1, "Cannot find current server", __FILE__, __FUNCTION__);
			exit(0);
		}

		const int nCurArea = m_pElementModule->GetPropertyInt32(*itr, excel::Server::Area());

		for (int i = 0; i < strIdList.size(); ++i)
		{
			const std::string& strId = strIdList[i];

			const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
			const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
			const int nServerArea = m_pElementModule->GetPropertyInt32(strId, excel::Server::Area());
			if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_WORLD && nCurArea == nServerArea)
			{
				const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
				//const int maxConnect = m_pElementModule->GetPropertyInt32(strId, SquickProtocol::Server::MaxOnline());
				//const int nCpus = m_pElementModule->GetPropertyInt32(strId, SquickProtocol::Server::CpuCount());
				const std::string& name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
				const std::string& ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

				ConnectData xServerData;

				xServerData.nGameID = serverID;
				xServerData.eServerType = (SQUICK_SERVER_TYPES)serverType;
				xServerData.ip = ip;
				xServerData.nPort = nPort;
				xServerData.name = strId;

				m_pNetClientModule->AddServer(xServerData);
			}
		}
	}

	return true;
}

void GameServerToWorldModule::OnServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
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

		//type
		ConnectData xServerData;

		xServerData.nGameID = xData.server_id();
		xServerData.ip = xData.server_ip();
		xServerData.nPort = xData.server_port();
		xServerData.name = xData.server_name();
		xServerData.nWorkLoad = xData.server_cur_count();
		xServerData.eServerType = (SQUICK_SERVER_TYPES)xData.server_type();

		if (SQUICK_SERVER_TYPES::SQUICK_ST_WORLD == xServerData.eServerType)
		{
			m_pNetClientModule->AddServer(xServerData);
		}
		else if (SQUICK_SERVER_TYPES::SQUICK_ST_DB == xServerData.eServerType)
		{
			m_pNetClientModule->AddServer(xServerData);
		}
	}
}

void GameServerToWorldModule::OnSocketWSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet* pNet)
{
	if (eEvent & SQUICK_NET_EVENT_EOF)
	{
	}
	else if (eEvent & SQUICK_NET_EVENT_ERROR)
	{
	}
	else if (eEvent & SQUICK_NET_EVENT_TIMEOUT)
	{
	}
	else  if (eEvent & SQUICK_NET_EVENT_CONNECTED)
	{
		m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
		Register(pNet);

	}
}

int GameServerToWorldModule::OnObjectClassEvent(const Guid& self, const std::string& className, const CLASS_OBJECT_EVENT classEvent, const DataList& var)
{
	if (className == excel::Player::ThisName())
	{
		if (CLASS_OBJECT_EVENT::COE_DESTROY == classEvent)
		{
			SendOffline(self);
		}
		else if (CLASS_OBJECT_EVENT::COE_CREATE_FINISH == classEvent)
		{
			SendOnline(self);
		}
	}

	return 0;
}

void GameServerToWorldModule::SendOnline(const Guid& self)
{
	/*
	if (m_pKernelModule->ExistObject(self))
	{
		SquickStruct::RoleOnlineNotify xMsg;
		const int& gateID = m_pKernelModule->GetPropertyInt(self, SquickProtocol::Player::GateID());
		const std::string& playerName = m_pKernelModule->GetPropertyString(self, SquickProtocol::Player::Name());
		const int bp = m_pKernelModule->GetPropertyInt(self, SquickProtocol::Player::BattlePoint());

		*xMsg.mutable_self() = INetModule::StructToProtobuf(self);
		xMsg.set_game(pPluginManager->GetAppID());
		xMsg.set_proxy(gateID);
		xMsg.set_name(playerName);
		xMsg.set_bp(bp);

		m_pNetClientModule->SendToAllServerByPB(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ACK_ONLINE_NOTIFY, xMsg, self);
	}*/
}

void GameServerToWorldModule::SendOffline(const Guid& self)
{
	if (m_pKernelModule->ExistObject(self))
	{
		SquickStruct::AckPlayerOffline xMsg;

		//const Guid& xClan = m_pKernelModule->GetPropertyObject(self, SquickProtocol::Player::Clan_ID());

		*xMsg.mutable_self() = INetModule::StructToProtobuf(self);
		*xMsg.mutable_object() = INetModule::StructToProtobuf(Guid());
		xMsg.set_game(pPluginManager->GetAppID());
		xMsg.set_proxy(0);

		m_pNetClientModule->SendToAllServerByPB(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::ServerRPC::ACK_PLAYER_OFFLINE, xMsg, self);
	}
}

void GameServerToWorldModule::TransPBToProxy(const SQUICK_SOCKET sockIndex, const int msgID, const char* msg, const uint32_t len)
{
	Guid nPlayerID;
	std::string strData;
	if (!INetModule::ReceivePB( msgID, msg, len, strData, nPlayerID))
	{
		return;
	}

	m_pGameServerNet_ServerModule->SendMsgToGate(msgID, strData, nPlayerID);

	return;
}

void GameServerToWorldModule::TransmitToWorld(const int nHashKey, const int msgID, const google::protobuf::Message& xData)
{
	m_pNetClientModule->SendSuitByPB(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, nHashKey, msgID, xData);
}

void GameServerToWorldModule::OnWorldPropertyIntProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectPropertyInt)

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::PropertyInt &xProperty = xMsg.property_list().Get(i);
		m_pKernelModule->SetPropertyInt(nPlayerID, xProperty.property_name(), xProperty.data());
	}
}

void GameServerToWorldModule::OnWorldPropertyFloatProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectPropertyFloat)

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::PropertyFloat &xProperty = xMsg.property_list().Get(i);
		m_pKernelModule->SetPropertyFloat(nPlayerID, xProperty.property_name(), xProperty.data());
	}
}

void GameServerToWorldModule::OnWorldPropertyStringProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectPropertyString)

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::PropertyString &xProperty = xMsg.property_list().Get(i);
		m_pKernelModule->SetPropertyString(nPlayerID, xProperty.property_name(), xProperty.data());
	}
}

void GameServerToWorldModule::OnWorldPropertyObjectProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectPropertyObject)

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::PropertyObject &xProperty = xMsg.property_list().Get(i);
		m_pKernelModule->SetPropertyObject(nPlayerID, xProperty.property_name(), INetModule::ProtobufToStruct(xProperty.data()));
	}
}

void GameServerToWorldModule::OnWorldPropertyVector2Process(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectPropertyVector2)

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::PropertyVector2 &xProperty = xMsg.property_list().Get(i);
		m_pKernelModule->SetPropertyVector2(nPlayerID, xProperty.property_name(),  INetModule::ProtobufToStruct(xProperty.data()));
	}
}

void GameServerToWorldModule::OnWorldPropertyVector3Process(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectPropertyVector3)

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::PropertyVector3 &xProperty = xMsg.property_list().Get(i);
		m_pKernelModule->SetPropertyVector3(nPlayerID, xProperty.property_name(),  INetModule::ProtobufToStruct(xProperty.data()));
	}
}

void GameServerToWorldModule::OnWorldRecordEnterProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::MultiObjectRecordList)

	for (int playerIndex = 0; playerIndex < xMsg.multi_player_record_size(); playerIndex++)
	{
		const SquickStruct::ObjectRecordList& objectRecordList = xMsg.multi_player_record(playerIndex);
		for (int j = 0; j < objectRecordList.record_list_size(); ++j)
		{
			const SquickStruct::ObjectRecordBase& recordBase = objectRecordList.record_list(j);
			auto record = m_pKernelModule->FindRecord(nPlayerID, recordBase.record_name());
			if (record)
			{
				CommonRedisModule::ConvertPBToRecord(recordBase, record);
			}
		}
	}
}

void GameServerToWorldModule::OnWorldAddRowProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectRecordAddRow)

	auto pRecord = m_pKernelModule->FindRecord(nPlayerID, xMsg.record_name());
	if (pRecord)
	{
		for (int i = 0; i < xMsg.row_data_size(); i++)
		{
			const SquickStruct::RecordAddRowStruct &xAddRowStruct = xMsg.row_data().Get(i);
			int row = xAddRowStruct.row();

			std::map<int, SquickData> colDataMap;
			for (int j = 0; j < xAddRowStruct.record_int_list_size(); j++)
			{
				const SquickStruct::RecordInt &xRecordInt = xAddRowStruct.record_int_list().Get(j);
				SquickData data;
				data.SetInt(xRecordInt.data());
				colDataMap[xRecordInt.col()] = data;
			}

			for (int j = 0; j < xAddRowStruct.record_float_list_size(); j++)
			{
				const SquickStruct::RecordFloat &xRecordFloat = xAddRowStruct.record_float_list().Get(j);
				SquickData data;
				data.SetFloat(xRecordFloat.data());
				colDataMap[xRecordFloat.col()] = data;
			}
			for (int j = 0; j < xAddRowStruct.record_string_list_size(); j++)
			{
				const SquickStruct::RecordString &xRecordString = xAddRowStruct.record_string_list().Get(j);
				SquickData data;
				data.SetString(xRecordString.data());
				colDataMap[xRecordString.col()] = data;
			}
			for (int j = 0; j < xAddRowStruct.record_object_list_size(); j++)
			{
				const SquickStruct::RecordObject &xRecordObject = xAddRowStruct.record_object_list().Get(j);
				SquickData data;
				data.SetObject(INetModule::ProtobufToStruct(xRecordObject.data()));
				colDataMap[xRecordObject.col()] = data;
			}

			for (int j = 0; j < xAddRowStruct.record_vector2_list_size(); j++)
			{
				const SquickStruct::RecordVector2 &xRecordObject = xAddRowStruct.record_vector2_list().Get(j);
				SquickData data;
				data.SetVector2(INetModule::ProtobufToStruct(xRecordObject.data()));
				colDataMap[xRecordObject.col()] = data;
			}

			for (int j = 0; j < xAddRowStruct.record_vector3_list_size(); j++)
			{
				const SquickStruct::RecordVector3 &xRecordObject = xAddRowStruct.record_vector3_list().Get(j);
				SquickData data;
				data.SetVector3(INetModule::ProtobufToStruct(xRecordObject.data()));
				colDataMap[xRecordObject.col()] = data;
			}

			DataList xDataList;
			for (int j = 0; j < colDataMap.size(); j++)
			{
				if (colDataMap.find(j) != colDataMap.end())
				{
					xDataList.Append(colDataMap[j]);
				}
				else
				{
					m_pLogModule->LogInfo(nPlayerID, "Upload From Client add row record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
					return;
				}
			}

			if (pRecord->AddRow(row, xDataList) >= 0)
			{
				m_pLogModule->LogInfo(nPlayerID, "Upload From Client add row record " + xMsg.record_name(), __FUNCTION__, __LINE__);
			}
			else
			{
				m_pLogModule->LogInfo(nPlayerID, "Upload From Client add row record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
			}
		}
	}
}

void GameServerToWorldModule::OnWorldRemoveRowProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectRecordRemove)

	auto pRecord = m_pKernelModule->FindRecord(nPlayerID, xMsg.record_name());
	if (pRecord)
	{
		for (int i = 0; i < xMsg.remove_row_size(); i++)
		{
			if (pRecord->Remove(xMsg.remove_row().Get(i)))
			{
				m_pLogModule->LogInfo(nPlayerID, "Upload From Client remove row record " + xMsg.record_name(), __FUNCTION__, __LINE__);
			}
			else
			{
				m_pLogModule->LogInfo(nPlayerID, "Upload From Client remove row record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
			}
		}
	}
}

void GameServerToWorldModule::OnWorldSwapRowProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{

}

void GameServerToWorldModule::OnWorldRecordIntProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectRecordInt)

	auto pRecord = m_pKernelModule->FindRecord(nPlayerID, xMsg.record_name());
	if (!pRecord)
	{
		m_pLogModule->LogError(nPlayerID, "Upload From Client int set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
		return;
	}

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::RecordInt &xRecordInt = xMsg.property_list().Get(i);
		pRecord->SetInt(xRecordInt.row(), xRecordInt.col(), xRecordInt.data());
		m_pLogModule->LogInfo(nPlayerID, "Upload From Client int set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
	}
}

void GameServerToWorldModule::OnWorldRecordFloatProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectRecordFloat)

	auto pRecord = m_pKernelModule->FindRecord(nPlayerID, xMsg.record_name());
	if (!pRecord)
	{
		m_pLogModule->LogError(nPlayerID, "Upload From Client float set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
		return;
	}

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::RecordFloat &xRecordFloat = xMsg.property_list().Get(i);
		pRecord->SetFloat(xRecordFloat.row(), xRecordFloat.col(), xRecordFloat.data());
		m_pLogModule->LogInfo(nPlayerID, "Upload From Client float set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
	}
}

void GameServerToWorldModule::OnWorldRecordStringProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectRecordString)

	auto pRecord = m_pKernelModule->FindRecord(nPlayerID, xMsg.record_name());
	if (!pRecord)
	{
		m_pLogModule->LogError(nPlayerID, "String set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
		return;
	}

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::RecordString &xRecordString = xMsg.property_list().Get(i);
		pRecord->SetString(xRecordString.row(), xRecordString.col(), xRecordString.data());
		m_pLogModule->LogInfo(nPlayerID, "String set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
	}
}

void GameServerToWorldModule::OnWorldRecordObjectProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectRecordObject)

	auto pRecord = m_pKernelModule->FindRecord(nPlayerID, xMsg.record_name());
	if (!pRecord)
	{
		m_pLogModule->LogError(nPlayerID, "Upload From Client Object set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
		return;
	}

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::RecordObject &xRecordObject = xMsg.property_list().Get(i);
		pRecord->SetObject(xRecordObject.row(), xRecordObject.col(), INetModule::ProtobufToStruct(xRecordObject.data()));
		m_pLogModule->LogInfo(nPlayerID, "Upload From Client Object set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
	}
}

void GameServerToWorldModule::OnWorldRecordVector2Process(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectRecordVector2)

	auto pRecord = m_pKernelModule->FindRecord(nPlayerID, xMsg.record_name());
	if (!pRecord)
	{
		m_pLogModule->LogError(nPlayerID, "Upload From Client vector2 set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
		return;
	}

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::RecordVector2 &xRecordVector2 = xMsg.property_list().Get(i);
		pRecord->SetVector2(xRecordVector2.row(), xRecordVector2.col(), INetModule::ProtobufToStruct(xRecordVector2.data()));
		m_pLogModule->LogInfo(nPlayerID, "Upload From Client vector2 set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
	}
}

void GameServerToWorldModule::OnWorldRecordVector3Process(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len)
{
	CLIENT_MSG_PROCESS( msgID, msg, len, SquickStruct::ObjectRecordVector3)

	auto pRecord = m_pKernelModule->FindRecord(nPlayerID, xMsg.record_name());
	if (!pRecord)
	{
		m_pLogModule->LogError(nPlayerID, "Upload From Client vector3 set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
		return;
	}

	for (int i = 0; i < xMsg.property_list_size(); i++)
	{
		const SquickStruct::RecordVector3 &xRecordVector3 = xMsg.property_list().Get(i);
		pRecord->SetVector3(xRecordVector3.row(), xRecordVector3.col(), INetModule::ProtobufToStruct(xRecordVector3.data()));
		m_pLogModule->LogInfo(nPlayerID, "Upload From Client vector3 set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
	}
}
