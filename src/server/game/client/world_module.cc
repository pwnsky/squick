

#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include "plugin.h"
#include "world_module.h"

#include <server/db_proxy/logic/common_redis_module.h>

bool GameServerToWorldModule::Start() {
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_pGameServerNet_ServerModule = pm_->FindModule<IGameServerNet_ServerModule>();

    return true;
}

bool GameServerToWorldModule::Destory() { return true; }

bool GameServerToWorldModule::Update() {
    ServerReport();
    return true;
}

void GameServerToWorldModule::Register(INet *pNet) {
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::SQUICK_ST_GAME && pm_->GetAppID() == serverID) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                // const int nCpus = m_element_->GetPropertyInt32(strId, SquickProtocol::Server::CpuCount());
                const std::string &name = m_element_->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_element_->GetPropertyString(strId, excel::Server::IP());

                SquickStruct::ServerInfoReportList xMsg;
                SquickStruct::ServerInfoReport *pData = xMsg.add_server_list();

                pData->set_server_id(serverID);
                pData->set_server_name(strId);
                pData->set_server_cur_count(0);
                pData->set_server_ip(ip);
                pData->set_server_port(nPort);
                pData->set_server_max_online(maxConnect);
                pData->set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
                pData->set_server_type(serverType);

                std::shared_ptr<ConnectData> pServerData = m_net_client_->GetServerNetInfo(pNet);
                if (pServerData) {
                    int nTargetID = pServerData->nGameID;
                    m_net_client_->SendToServerByPB(nTargetID, SquickStruct::ServerRPC::GAME_TO_WORLD_REGISTERED, xMsg);

                    m_log_->LogInfo(Guid(0, pData->server_id()), pData->server_name(), "Register");
                }
            }
        }
    }
}

void GameServerToWorldModule::ServerReport() {
    if (mLastReportTime + 10 > pm_->GetNowTime()) {
        return;
    }
    mLastReportTime = pm_->GetNowTime();

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (pm_->GetAppID() == serverID) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const std::string &name = m_element_->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_element_->GetPropertyString(strId, excel::Server::IP());

                SquickStruct::ServerInfoReport reqMsg;

                reqMsg.set_server_id(serverID);
                reqMsg.set_server_name(strId);
                reqMsg.set_server_cur_count(m_kernel_->GetOnLineCount());
                reqMsg.set_server_ip(ip);
                reqMsg.set_server_port(nPort);
                reqMsg.set_server_max_online(maxConnect);
                reqMsg.set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
                reqMsg.set_server_type(serverType);

                m_net_client_->SendToAllServerByPB(ServerType::SQUICK_ST_WORLD, SquickStruct::STS_SERVER_REPORT, reqMsg, Guid());
            }
        }
    }
}

bool GameServerToWorldModule::AfterStart() {
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_INT, this,
                                           &GameServerToWorldModule::OnWorldPropertyIntProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_FLOAT, this,
                                           &GameServerToWorldModule::OnWorldPropertyFloatProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_STRING, this,
                                           &GameServerToWorldModule::OnWorldPropertyStringProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_OBJECT, this,
                                           &GameServerToWorldModule::OnWorldPropertyObjectProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_VECTOR2, this,
                                           &GameServerToWorldModule::OnWorldPropertyVector2Process);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_PROPERTY_VECTOR3, this,
                                           &GameServerToWorldModule::OnWorldPropertyVector3Process);

    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_OBJECT_RECORD_ENTRY, this,
                                           &GameServerToWorldModule::OnWorldRecordEnterProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_ADD_ROW, this,
                                           &GameServerToWorldModule::OnWorldAddRowProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_REMOVE_ROW, this,
                                           &GameServerToWorldModule::OnWorldRemoveRowProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_SWAP_ROW, this,
                                           &GameServerToWorldModule::OnWorldSwapRowProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_INT, this,
                                           &GameServerToWorldModule::OnWorldRecordIntProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_FLOAT, this,
                                           &GameServerToWorldModule::OnWorldRecordFloatProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_STRING, this,
                                           &GameServerToWorldModule::OnWorldRecordStringProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_OBJECT, this,
                                           &GameServerToWorldModule::OnWorldRecordObjectProcess);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_VECTOR2, this,
                                           &GameServerToWorldModule::OnWorldRecordVector2Process);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_RECORD_VECTOR3, this,
                                           &GameServerToWorldModule::OnWorldRecordVector3Process);

    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, SquickStruct::STS_NET_INFO, this,
                                           &GameServerToWorldModule::OnServerInfoProcess);

    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, this, &GameServerToWorldModule::TransPBToProxy);

    m_net_client_->AddEventCallBack(ServerType::SQUICK_ST_WORLD, this, &GameServerToWorldModule::OnSocketWSEvent);

    m_kernel_->AddClassCallBack(excel::Player::ThisName(), this, &GameServerToWorldModule::OnObjectClassEvent);

    m_net_client_->ExpandBufferSize();

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();

        const int nCurAppID = pm_->GetAppID();
        std::vector<std::string>::const_iterator itr = std::find_if(strIdList.begin(), strIdList.end(), [&](const std::string &strConfigId) {
            return nCurAppID == m_element_->GetPropertyInt32(strConfigId, excel::Server::ServerID());
        });

        if (strIdList.end() == itr) {
            std::ostringstream strLog;
            strLog << "Cannot find current server, AppID = " << nCurAppID;
            m_log_->LogError(NULL_OBJECT, strLog, __FILE__, __LINE__);
            SQUICK_ASSERT(-1, "Cannot find current server", __FILE__, __FUNCTION__);
            exit(0);
        }

        const int nCurArea = m_element_->GetPropertyInt32(*itr, excel::Server::Area());

        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            const int nServerArea = m_element_->GetPropertyInt32(strId, excel::Server::Area());
            if (serverType == ServerType::SQUICK_ST_WORLD && nCurArea == nServerArea) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                // const int maxConnect = m_element_->GetPropertyInt32(strId, SquickProtocol::Server::MaxOnline());
                // const int nCpus = m_element_->GetPropertyInt32(strId, SquickProtocol::Server::CpuCount());
                const std::string &name = m_element_->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_element_->GetPropertyString(strId, excel::Server::IP());

                ConnectData xServerData;

                xServerData.nGameID = serverID;
                xServerData.eServerType = (ServerType)serverType;
                xServerData.ip = ip;
                xServerData.nPort = nPort;
                xServerData.name = strId;

                m_net_client_->AddServer(xServerData);
            }
        }
    }

    return true;
}

void GameServerToWorldModule::OnServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!INetModule::ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);

        // type
        ConnectData xServerData;

        xServerData.nGameID = xData.server_id();
        xServerData.ip = xData.server_ip();
        xServerData.nPort = xData.server_port();
        xServerData.name = xData.server_name();
        xServerData.nWorkLoad = xData.server_cur_count();
        xServerData.eServerType = (ServerType)xData.server_type();

        if (ServerType::SQUICK_ST_WORLD == xServerData.eServerType) {
            m_net_client_->AddServer(xServerData);
        } else if (ServerType::SQUICK_ST_DB_PROXY == xServerData.eServerType) {
            m_net_client_->AddServer(xServerData);
        }
    }
}

void GameServerToWorldModule::OnSocketWSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
        Register(pNet);
    }
}

int GameServerToWorldModule::OnObjectClassEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var) {
    if (className == excel::Player::ThisName()) {
        if (CLASS_OBJECT_EVENT::COE_DESTROY == classEvent) {
            SendOffline(self);
        } else if (CLASS_OBJECT_EVENT::COE_CREATE_FINISH == classEvent) {
            SendOnline(self);
        }
    }

    return 0;
}

void GameServerToWorldModule::SendOnline(const Guid &self) {
    /*
    if (m_kernel_->ExistObject(self))
    {
            SquickStruct::RoleOnlineNotify xMsg;
            const int& gateID = m_kernel_->GetPropertyInt(self, SquickProtocol::Player::GateID());
            const std::string& playerName = m_kernel_->GetPropertyString(self, SquickProtocol::Player::Name());
            const int bp = m_kernel_->GetPropertyInt(self, SquickProtocol::Player::BattlePoint());

            *xMsg.mutable_self() = INetModule::StructToProtobuf(self);
            xMsg.set_game(pm_->GetAppID());
            xMsg.set_proxy(gateID);
            xMsg.set_name(playerName);
            xMsg.set_bp(bp);

            m_net_client_->SendToAllServerByPB(ServerType::SQUICK_ST_WORLD, SquickStruct::ACK_ONLINE_NOTIFY, xMsg, self);
    }*/
}

void GameServerToWorldModule::SendOffline(const Guid &self) {
    if (m_kernel_->ExistObject(self)) {
        SquickStruct::AckPlayerOffline xMsg;

        // const Guid& xClan = m_kernel_->GetPropertyObject(self, SquickProtocol::Player::Clan_ID());

        *xMsg.mutable_self() = INetModule::StructToProtobuf(self);
        *xMsg.mutable_object() = INetModule::StructToProtobuf(Guid());
        xMsg.set_game(pm_->GetAppID());
        xMsg.set_proxy(0);

        m_net_client_->SendToAllServerByPB(ServerType::SQUICK_ST_WORLD, SquickStruct::ServerRPC::ACK_PLAYER_OFFLINE, xMsg, self);
    }
}

void GameServerToWorldModule::TransPBToProxy(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    std::string strData;
    if (!INetModule::ReceivePB(msg_id, msg, len, strData, nPlayerID)) {
        return;
    }

    m_pGameServerNet_ServerModule->SendMsgToProxy(msg_id, strData, nPlayerID);

    return;
}

void GameServerToWorldModule::TransmitToWorld(const int nHashKey, const int msg_id, const google::protobuf::Message &xData) {
    m_net_client_->SendSuitByPB(ServerType::SQUICK_ST_WORLD, nHashKey, msg_id, xData);
}

void GameServerToWorldModule::OnWorldPropertyIntProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectPropertyInt)

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::PropertyInt &xProperty = xMsg.property_list().Get(i);
        m_kernel_->SetPropertyInt(nPlayerID, xProperty.property_name(), xProperty.data());
    }
}

void GameServerToWorldModule::OnWorldPropertyFloatProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectPropertyFloat)

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::PropertyFloat &xProperty = xMsg.property_list().Get(i);
        m_kernel_->SetPropertyFloat(nPlayerID, xProperty.property_name(), xProperty.data());
    }
}

void GameServerToWorldModule::OnWorldPropertyStringProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectPropertyString)

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::PropertyString &xProperty = xMsg.property_list().Get(i);
        m_kernel_->SetPropertyString(nPlayerID, xProperty.property_name(), xProperty.data());
    }
}

void GameServerToWorldModule::OnWorldPropertyObjectProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectPropertyObject)

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::PropertyObject &xProperty = xMsg.property_list().Get(i);
        m_kernel_->SetPropertyObject(nPlayerID, xProperty.property_name(), INetModule::ProtobufToStruct(xProperty.data()));
    }
}

void GameServerToWorldModule::OnWorldPropertyVector2Process(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectPropertyVector2)

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::PropertyVector2 &xProperty = xMsg.property_list().Get(i);
        m_kernel_->SetPropertyVector2(nPlayerID, xProperty.property_name(), INetModule::ProtobufToStruct(xProperty.data()));
    }
}

void GameServerToWorldModule::OnWorldPropertyVector3Process(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectPropertyVector3)

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::PropertyVector3 &xProperty = xMsg.property_list().Get(i);
        m_kernel_->SetPropertyVector3(nPlayerID, xProperty.property_name(), INetModule::ProtobufToStruct(xProperty.data()));
    }
}

void GameServerToWorldModule::OnWorldRecordEnterProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::MultiObjectRecordList)

    for (int playerIndex = 0; playerIndex < xMsg.multi_player_record_size(); playerIndex++) {
        const SquickStruct::ObjectRecordList &objectRecordList = xMsg.multi_player_record(playerIndex);
        for (int j = 0; j < objectRecordList.record_list_size(); ++j) {
            const SquickStruct::ObjectRecordBase &recordBase = objectRecordList.record_list(j);
            auto record = m_kernel_->FindRecord(nPlayerID, recordBase.record_name());
            if (record) {
                CommonRedisModule::ConvertPBToRecord(recordBase, record);
            }
        }
    }
}

void GameServerToWorldModule::OnWorldAddRowProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectRecordAddRow)

    auto pRecord = m_kernel_->FindRecord(nPlayerID, xMsg.record_name());
    if (pRecord) {
        for (int i = 0; i < xMsg.row_data_size(); i++) {
            const SquickStruct::RecordAddRowStruct &xAddRowStruct = xMsg.row_data().Get(i);
            int row = xAddRowStruct.row();

            std::map<int, SquickData> colDataMap;
            for (int j = 0; j < xAddRowStruct.record_int_list_size(); j++) {
                const SquickStruct::RecordInt &xRecordInt = xAddRowStruct.record_int_list().Get(j);
                SquickData data;
                data.SetInt(xRecordInt.data());
                colDataMap[xRecordInt.col()] = data;
            }

            for (int j = 0; j < xAddRowStruct.record_float_list_size(); j++) {
                const SquickStruct::RecordFloat &xRecordFloat = xAddRowStruct.record_float_list().Get(j);
                SquickData data;
                data.SetFloat(xRecordFloat.data());
                colDataMap[xRecordFloat.col()] = data;
            }
            for (int j = 0; j < xAddRowStruct.record_string_list_size(); j++) {
                const SquickStruct::RecordString &xRecordString = xAddRowStruct.record_string_list().Get(j);
                SquickData data;
                data.SetString(xRecordString.data());
                colDataMap[xRecordString.col()] = data;
            }
            for (int j = 0; j < xAddRowStruct.record_object_list_size(); j++) {
                const SquickStruct::RecordObject &xRecordObject = xAddRowStruct.record_object_list().Get(j);
                SquickData data;
                data.SetObject(INetModule::ProtobufToStruct(xRecordObject.data()));
                colDataMap[xRecordObject.col()] = data;
            }

            for (int j = 0; j < xAddRowStruct.record_vector2_list_size(); j++) {
                const SquickStruct::RecordVector2 &xRecordObject = xAddRowStruct.record_vector2_list().Get(j);
                SquickData data;
                data.SetVector2(INetModule::ProtobufToStruct(xRecordObject.data()));
                colDataMap[xRecordObject.col()] = data;
            }

            for (int j = 0; j < xAddRowStruct.record_vector3_list_size(); j++) {
                const SquickStruct::RecordVector3 &xRecordObject = xAddRowStruct.record_vector3_list().Get(j);
                SquickData data;
                data.SetVector3(INetModule::ProtobufToStruct(xRecordObject.data()));
                colDataMap[xRecordObject.col()] = data;
            }

            DataList xDataList;
            for (int j = 0; j < colDataMap.size(); j++) {
                if (colDataMap.find(j) != colDataMap.end()) {
                    xDataList.Append(colDataMap[j]);
                } else {
                    m_log_->LogInfo(nPlayerID, "Upload From Client add row record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
                    return;
                }
            }

            if (pRecord->AddRow(row, xDataList) >= 0) {
                m_log_->LogInfo(nPlayerID, "Upload From Client add row record " + xMsg.record_name(), __FUNCTION__, __LINE__);
            } else {
                m_log_->LogInfo(nPlayerID, "Upload From Client add row record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
            }
        }
    }
}

void GameServerToWorldModule::OnWorldRemoveRowProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectRecordRemove)

    auto pRecord = m_kernel_->FindRecord(nPlayerID, xMsg.record_name());
    if (pRecord) {
        for (int i = 0; i < xMsg.remove_row_size(); i++) {
            if (pRecord->Remove(xMsg.remove_row().Get(i))) {
                m_log_->LogInfo(nPlayerID, "Upload From Client remove row record " + xMsg.record_name(), __FUNCTION__, __LINE__);
            } else {
                m_log_->LogInfo(nPlayerID, "Upload From Client remove row record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
            }
        }
    }
}

void GameServerToWorldModule::OnWorldSwapRowProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void GameServerToWorldModule::OnWorldRecordIntProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectRecordInt)

    auto pRecord = m_kernel_->FindRecord(nPlayerID, xMsg.record_name());
    if (!pRecord) {
        m_log_->LogError(nPlayerID, "Upload From Client int set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
        return;
    }

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::RecordInt &xRecordInt = xMsg.property_list().Get(i);
        pRecord->SetInt(xRecordInt.row(), xRecordInt.col(), xRecordInt.data());
        m_log_->LogInfo(nPlayerID, "Upload From Client int set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
    }
}

void GameServerToWorldModule::OnWorldRecordFloatProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectRecordFloat)

    auto pRecord = m_kernel_->FindRecord(nPlayerID, xMsg.record_name());
    if (!pRecord) {
        m_log_->LogError(nPlayerID, "Upload From Client float set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
        return;
    }

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::RecordFloat &xRecordFloat = xMsg.property_list().Get(i);
        pRecord->SetFloat(xRecordFloat.row(), xRecordFloat.col(), xRecordFloat.data());
        m_log_->LogInfo(nPlayerID, "Upload From Client float set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
    }
}

void GameServerToWorldModule::OnWorldRecordStringProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectRecordString)

    auto pRecord = m_kernel_->FindRecord(nPlayerID, xMsg.record_name());
    if (!pRecord) {
        m_log_->LogError(nPlayerID, "String set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
        return;
    }

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::RecordString &xRecordString = xMsg.property_list().Get(i);
        pRecord->SetString(xRecordString.row(), xRecordString.col(), xRecordString.data());
        m_log_->LogInfo(nPlayerID, "String set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
    }
}

void GameServerToWorldModule::OnWorldRecordObjectProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectRecordObject)

    auto pRecord = m_kernel_->FindRecord(nPlayerID, xMsg.record_name());
    if (!pRecord) {
        m_log_->LogError(nPlayerID, "Upload From Client Object set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
        return;
    }

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::RecordObject &xRecordObject = xMsg.property_list().Get(i);
        pRecord->SetObject(xRecordObject.row(), xRecordObject.col(), INetModule::ProtobufToStruct(xRecordObject.data()));
        m_log_->LogInfo(nPlayerID, "Upload From Client Object set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
    }
}

void GameServerToWorldModule::OnWorldRecordVector2Process(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectRecordVector2)

    auto pRecord = m_kernel_->FindRecord(nPlayerID, xMsg.record_name());
    if (!pRecord) {
        m_log_->LogError(nPlayerID, "Upload From Client vector2 set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
        return;
    }

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::RecordVector2 &xRecordVector2 = xMsg.property_list().Get(i);
        pRecord->SetVector2(xRecordVector2.row(), xRecordVector2.col(), INetModule::ProtobufToStruct(xRecordVector2.data()));
        m_log_->LogInfo(nPlayerID, "Upload From Client vector2 set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
    }
}

void GameServerToWorldModule::OnWorldRecordVector3Process(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    CLIENT_MSG_PROCESS(msg_id, msg, len, SquickStruct::ObjectRecordVector3)

    auto pRecord = m_kernel_->FindRecord(nPlayerID, xMsg.record_name());
    if (!pRecord) {
        m_log_->LogError(nPlayerID, "Upload From Client vector3 set record error " + xMsg.record_name(), __FUNCTION__, __LINE__);
        return;
    }

    for (int i = 0; i < xMsg.property_list_size(); i++) {
        const SquickStruct::RecordVector3 &xRecordVector3 = xMsg.property_list().Get(i);
        pRecord->SetVector3(xRecordVector3.row(), xRecordVector3.col(), INetModule::ProtobufToStruct(xRecordVector3.data()));
        m_log_->LogInfo(nPlayerID, "Upload From Client vector3 set record " + xMsg.record_name(), __FUNCTION__, __LINE__);
    }
}
