
#include "world_module.h"
#include "plugin.h"
#include <squick/struct/struct.h>
namespace gameplay_manager::client {
bool WorldModule::Start() {
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_log_ = pm_->FindModule<ILogModule>();

    return true;
}

bool WorldModule::Destory() { return true; }

bool WorldModule::AfterStart() {
    m_net_client_->AddEventCallBack(ServerType::ST_WORLD, this, &WorldModule::OnSocketMSEvent);
    m_net_client_->AddReceiveCallBack(ServerType::ST_WORLD, SquickStruct::STS_NET_INFO, this, &WorldModule::OnServerInfoProcess);
    m_net_client_->ExpandBufferSize();

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::ST_WORLD) {
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

bool WorldModule::BeforeDestory() { return false; }

bool WorldModule::Update() {
    ServerReport();
    return true;
}

void WorldModule::Register(INet *pNet) {

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::ST_GAMEPLAY_MANAGER && pm_->GetAppID() == serverID) {
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
                    m_net_client_->SendToServerByPB(nTargetID, SquickStruct::ServerRPC::GAMEPLAY_MANAGER_TO_WORLD_REGISTERED, xMsg);

                    m_log_->LogInfo(Guid(0, pData->server_id()), pData->server_name(), "Register");
                }
            }
        }
    }
}

// 处理来自World的服务器增加请求
void WorldModule::OnServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
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

        switch (xServerData.eServerType) {
        case ServerType::ST_GAME: {
            m_net_client_->AddServer(xServerData);
        } break;
        case ServerType::ST_WORLD: {
            m_net_client_->AddServer(xServerData);
        } break;
        default:
            break;
        }
    }
}

void WorldModule::ServerReport() {

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

                m_net_client_->SendToAllServerByPB(ServerType::ST_WORLD, SquickStruct::STS_SERVER_REPORT, reqMsg, Guid());
            }
        }
    }
}

void WorldModule::OnSocketMSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
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

INetClientModule *WorldModule::GetClusterModule() { return m_net_client_; }

} // namespace gameplay_manager::client