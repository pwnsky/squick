

#include "master_module.h"
#include "plugin.h"
#include <squick/core/data_list.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/struct/struct.h>

bool WorldToMasterModule::Start() {
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_pWorldNet_ServerModule = pm_->FindModule<IWorldNet_ServerModule>();
    m_security_ = pm_->FindModule<ISecurityModule>();

    return true;
}

bool WorldToMasterModule::Destory() { return true; }

bool WorldToMasterModule::AfterStart() {
    // m_net_client_->AddReceiveCallBack(ServerType::ST_MASTER, SquickStruct::REQ_CONNECT_WORLD, this,
    // &WorldToMasterModule::OnSelectServerProcess); m_net_client_->AddReceiveCallBack(ServerType::ST_MASTER,
    // SquickStruct::REQ_KICKED_FROM_WORLD, this, &WorldToMasterModule::OnKickClientProcess);
    m_net_client_->AddReceiveCallBack(ServerType::ST_MASTER, this, &WorldToMasterModule::InvalidMessage);

    m_net_client_->AddReceiveCallBack(ServerType::ST_MASTER, SquickStruct::STS_NET_INFO, this, &WorldToMasterModule::OnServerInfoProcess);

    m_net_client_->AddEventCallBack(ServerType::ST_MASTER, this, &WorldToMasterModule::OnSocketMSEvent);
    m_net_client_->ExpandBufferSize();

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::ST_MASTER) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_element_->GetPropertyInt32(strId, excel::Server::CpuCount());
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

bool WorldToMasterModule::Update() {
    // SQUICK_WILL_DO
    ServerReport();
    return true;
}

void WorldToMasterModule::Register(INet *pNet) {
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::ST_WORLD && pm_->GetAppID() == serverID) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_element_->GetPropertyInt32(strId, excel::Server::CpuCount());
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
                    m_net_client_->SendToServerByPB(nTargetID, SquickStruct::ServerRPC::WORLD_TO_MASTER_REGISTERED, xMsg);

                    m_log_->LogInfo(Guid(0, pData->server_id()), pData->server_name(), "Register");
                }
            }
        }
    }
}

void WorldToMasterModule::ServerReport() {
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
                reqMsg.set_server_cur_count(0);
                reqMsg.set_server_ip(ip);
                reqMsg.set_server_port(nPort);
                reqMsg.set_server_max_online(maxConnect);
                reqMsg.set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
                reqMsg.set_server_type(serverType);
                dout << "World Server report: " << serverType << std::endl;
                m_net_client_->SendToAllServerByPB(ServerType::ST_MASTER, SquickStruct::STS_SERVER_REPORT, reqMsg, Guid());
            }
        }
    }
}

void WorldToMasterModule::RefreshWorldInfo() {}

void WorldToMasterModule::OnSelectServerProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    /*
    Guid nPlayerID;
    SquickStruct::ReqConnectWorld xMsg;
    if (!INetModule::ReceivePB( msg_id, msg, len, xMsg, nPlayerID))
    {
            return;
    }

    std::shared_ptr<ServerData> xServerData = m_pWorldNet_ServerModule->GetSuitProxyToEnter();
    if (xServerData)
    {
            const std::string& strSecurityKey = m_security_->GetSecurityKey(xMsg.account());

            SquickStruct::AckConnectWorldResult xData;

            xData.set_world_id(xMsg.world_id());
            xData.mutable_sender()->CopyFrom(xMsg.sender());
            xData.set_login_id(xMsg.login_id());
            xData.set_account(xMsg.account());

            xData.set_world_ip(xServerData->pData->server_ip());
            xData.set_world_port(xServerData->pData->server_port());
            xData.set_world_key(strSecurityKey);

            m_net_->SendMsgPB(SquickStruct::ACK_CONNECT_WORLD, xData, xServerData->nFD);
            m_net_client_->SendSuitByPB(ServerType::ST_MASTER, xMsg.account(), SquickStruct::ACK_CONNECT_WORLD, xData);
    }*/
}

void WorldToMasterModule::OnKickClientProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void WorldToMasterModule::InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    printf("Net || umsg_id=%d\n", msg_id);
}

void WorldToMasterModule::OnSocketMSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
        Register(pNet);
    }
}

void WorldToMasterModule::OnClientDisconnect(const socket_t sock) {}

void WorldToMasterModule::OnClientConnected(const socket_t sock) {}

bool WorldToMasterModule::BeforeDestory() { return true; }

void WorldToMasterModule::LogServerInfo(const std::string &strServerInfo) { m_log_->LogInfo(Guid(), strServerInfo, ""); }

void WorldToMasterModule::OnServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    m_pWorldNet_ServerModule->OnServerInfoProcess(sock, msg_id, msg, len);
}
