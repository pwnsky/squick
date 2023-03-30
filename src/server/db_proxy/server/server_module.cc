#include <squick/struct/struct.h>

#include "../logic/player_redis_module.h"
#include "server_module.h"
bool DBNet_ServerModule::Awake() {
    this->pm_->SetAppType(ServerType::ST_DB_PROXY);

    return true;
}

bool DBNet_ServerModule::Start() {
    m_net_ = pm_->FindModule<INetModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_pAccountRedisModule = pm_->FindModule<IAccountRedisModule>();
    m_pPlayerRedisModule = pm_->FindModule<IPlayerRedisModule>();
    m_pThreadPoolModule = pm_->FindModule<IThreadPoolModule>();

    return true;
}

bool DBNet_ServerModule::AfterStart() {
    m_net_->AddEventCallBack(this, &DBNet_ServerModule::OnSocketEvent);
    m_net_->ExpandBufferSize();

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::ST_DB_PROXY && pm_->GetAppID() == serverID) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_element_->GetPropertyInt32(strId, excel::Server::CpuCount());
                // const std::string& name = m_element_->GetPropertyString(strId, SquickProtocol::Server::ID());
                // const std::string& ip = m_element_->GetPropertyString(strId, SquickProtocol::Server::IP());

                int nRet = m_net_->Startialization(maxConnect, nPort, nCpus);
                if (nRet < 0) {
                    std::ostringstream strLog;
                    strLog << "Cannot init server net, Port = " << nPort;
                    m_log_->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                    SQUICK_ASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
                    exit(0);
                }
            }
        }
    }

    m_net_->AddReceiveCallBack(SquickStruct::DbProxyRPC::REQ_PLAYER_LIST, this, &DBNet_ServerModule::OnRequireRoleListProcess);

    m_net_->AddReceiveCallBack(SquickStruct::DbProxyRPC::REQ_PLAYER_CREATE, this, &DBNet_ServerModule::OnCreateRoleGameProcess);
    m_net_->AddReceiveCallBack(SquickStruct::DbProxyRPC::REQ_PLAYER_DELETE, this, &DBNet_ServerModule::OnDeleteRoleGameProcess);
    m_net_->AddReceiveCallBack(SquickStruct::DbProxyRPC::REQ_PLAYER_DATA_LOAD, this, &DBNet_ServerModule::OnLoadRoleDataProcess);
    m_net_->AddReceiveCallBack(SquickStruct::DbProxyRPC::REQ_PLAYER_DATA_SAVE, this, &DBNet_ServerModule::OnSaveRoleDataProcess);

    return true;
}

bool DBNet_ServerModule::Destory() { return true; }

bool DBNet_ServerModule::Update() { return true; }

// Socket 事件
void DBNet_ServerModule::OnSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
        OnClientDisconnect(sock);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
        OnClientDisconnect(sock);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
        OnClientDisconnect(sock);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
        OnClientConnected(sock);
    }
}

void DBNet_ServerModule::OnClientDisconnect(const socket_t sock) {}

void DBNet_ServerModule::OnClientConnected(const socket_t sock) {}

// 获取角色列表
void DBNet_ServerModule::OnRequireRoleListProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    /*
    Guid clientID;
    SquickStruct::ReqRoleList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID))
    {
            return;
    }

    Guid xPlayerID;
    std::string strRoleName;
    if (!m_pPlayerRedisModule->GetRoleInfo(xMsg.account(), strRoleName, xPlayerID))
    {
            SquickStruct::AckRoleLiteInfoList xAckRoleLiteInfoList;
            xAckRoleLiteInfoList.set_account(xMsg.account());
            m_net_->SendMsgPB(SquickStruct::ACK_ROLE_LIST, xAckRoleLiteInfoList, sock, clientID);
            return;
    }

    SquickStruct::AckRoleLiteInfoList xAckRoleLiteInfoList;
    xAckRoleLiteInfoList.set_account(xMsg.account());

    SquickStruct::RoleLiteInfo* pData = xAckRoleLiteInfoList.add_char_data();
    pData->mutable_id()->CopyFrom(INetModule::StructToProtobuf(xPlayerID));
    pData->set_game_id(pm_->GetAppID());
    pData->set_last_offline_time(0);
    pData->set_last_offline_ip(0);
    pData->set_view_record("");

    m_net_->SendMsgPB(SquickStruct::ACK_ROLE_LIST, xAckRoleLiteInfoList, sock, clientID);
    */
}

void DBNet_ServerModule::OnCreateRoleGameProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    /*
    Guid clientID;
    SquickStruct::ReqCreateRole xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID))
    {
            return;
    }

    const std::string& account = xMsg.account();
    const std::string& name = xMsg.name();
    const int nHomeSceneID = 1;
  #ifdef SQUICK_DEV
    std::cout << "DBNet_ServerModule::OnCreateRoleGameProcess account: " << account << std::endl;
  #endif
    Guid xID = m_kernel_->CreateGUID();

    if (m_pPlayerRedisModule->CreateRole(account, name, xID, nHomeSceneID))
    {
            SquickStruct::AckRoleLiteInfoList xAckRoleLiteInfoList;
            xAckRoleLiteInfoList.set_account(account);

            SquickStruct::RoleLiteInfo* pData = xAckRoleLiteInfoList.add_char_data();
            pData->mutable_id()->CopyFrom(INetModule::StructToProtobuf(xID));
            pData->set_game_id(pm_->GetAppID());
            pData->set_last_offline_time(0);
            pData->set_last_offline_ip(0);
            pData->set_view_record("");

  #ifdef SQUICK_DEV
            std::cout << "创建角色成功 " << std::endl;
            std::cout << "响应给客户端 SquickStruct::ACK_ROLE_LIST : clienId" << std::endl;
  #endif
            // 响应创建角色
            m_net_->SendMsgPB(SquickStruct::ACK_ROLE_LIST, xAckRoleLiteInfoList, sock, clientID);
    }
    */
}

void DBNet_ServerModule::OnDeleteRoleGameProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    /*
    Guid clientID;
    SquickStruct::ReqDeleteRole xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID))
    {
            return;
    }

    SquickStruct::AckRoleLiteInfoList xAckRoleLiteInfoList;
    xAckRoleLiteInfoList.set_account(xMsg.account());

    m_net_->SendMsgPB(SquickStruct::ACK_ROLE_LIST, xAckRoleLiteInfoList, sock, clientID);

    */
}

void DBNet_ServerModule::OnLoadRoleDataProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid clientID;
    SquickStruct::ReqEnter xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID)) {
        return;
    }

    dout << clientID.ToString() << "请求加载角色数据\n";

    // Guid roleID = INetModule::ProtobufToStruct(xMsg.object());
    Guid xID = m_kernel_->CreateGUID();
    SquickStruct::PlayerData xPlayerData;
    xPlayerData.mutable_object()->CopyFrom(INetModule::StructToProtobuf(xID));

    // PlayerRedisModule* pPlayerRedisModule = (PlayerRedisModule*)m_pPlayerRedisModule;
    // pPlayerRedisModule->LoadPlayerData(roleID, xPlayerData);

    m_net_->SendMsgPB(SquickStruct::DbProxyRPC::ACK_PLAYER_DATA_LOAD, xPlayerData, sock, clientID);
}

void DBNet_ServerModule::OnSaveRoleDataProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    /*
    Guid clientID;
    SquickStruct::RoleDataPack xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID))
    {
            return;
    }

    Guid roleID = INetModule::ProtobufToStruct(xMsg.id());

    PlayerRedisModule* pPlayerRedisModule = (PlayerRedisModule*)m_pPlayerRedisModule;
    pPlayerRedisModule->SavePlayerData(roleID, xMsg);

    */
}
