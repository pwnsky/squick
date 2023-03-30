
#include <squick/plugin/config/i_class_module.h>

#include "plugin.h"
#include "world_module.h"
namespace proxy::client {
bool WorldModule::Start() {
    m_security_ = pm_->FindModule<ISecurityModule>();
    m_logic_ = pm_->FindModule<logic::ILogicModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    server_module_ = pm_->FindModule<server::IServerModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();

    return true;
}

bool WorldModule::Destory() {
    // Final();
    // Clear();
    return true;
}

bool WorldModule::Update() {
    ServerReport();
    return true;
}

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

void WorldModule::OnSocketWSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
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

void WorldModule::Register(INet *pNet) {
    dout << "注册代理服务器\n";
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::ST_PROXY && pm_->GetAppID() == serverID) {
                dout << "Register proxy server to world\n";
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

                std::shared_ptr<ConnectData> pServerData = GetClusterModule()->GetServerNetInfo(pNet);
                if (pServerData) {
                    int nTargetID = pServerData->nGameID;
                    GetClusterModule()->SendToServerByPB(nTargetID, SquickStruct::ServerRPC::PROXY_TO_WORLD_REGISTERED, xMsg);

                    m_log_->LogInfo(Guid(0, pData->server_id()), pData->server_name(), "Register");
                }
            }
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
                ////////cur count
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

bool WorldModule::AfterStart() {
    // m_net_client_->AddReceiveCallBack(ServerType::ST_WORLD, SquickStruct::ACK_CONNECT_WORLD, this,
    // &ProxyServerToWorldModule::OnSelectServerResultProcess);
    m_net_client_->AddReceiveCallBack(ServerType::ST_WORLD, SquickStruct::STS_NET_INFO, this, &WorldModule::OnServerInfoProcess);
    m_net_client_->AddReceiveCallBack(ServerType::ST_WORLD, this, &WorldModule::OnOtherMessage);
    m_net_client_->AddEventCallBack(ServerType::ST_WORLD, this, &WorldModule::OnSocketWSEvent);
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
            if (serverType == ServerType::ST_WORLD && nCurArea == nServerArea) {
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

void WorldModule::OnSelectServerResultProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    /*
    Guid nPlayerID;
    SquickStruct::AckConnectWorldResult xMsg;
    if (!INetModule::ReceivePB( msg_id, msg, len, xMsg, nPlayerID))
    {
        return;
    }

    std::shared_ptr<ClientConnectData> pConnectData = mWantToConnectMap.GetElement(xMsg.account());
    if (NULL != pConnectData)
    {
        pConnectData->strConnectKey = xMsg.world_key();
        return;
    }

    pConnectData = std::shared_ptr<ClientConnectData>(new ClientConnectData());
    pConnectData->account = xMsg.account();
    pConnectData->strConnectKey = xMsg.world_key();
    mWantToConnectMap.AddElement(pConnectData->account, pConnectData);

    */
}

INetClientModule *WorldModule::GetClusterModule() { return m_net_client_; }

bool WorldModule::VerifyConnectData(const std::string &account, const std::string &strKey) {
    std::shared_ptr<ClientConnectData> pConnectData = mWantToConnectMap.GetElement(account);
    if (pConnectData && strKey == pConnectData->strConnectKey) {
        mWantToConnectMap.RemoveElement(account);

        return true;
    }

    return false;
}

void WorldModule::LogServerInfo(const std::string &strServerInfo) { m_log_->LogInfo(Guid(), strServerInfo, ""); }

void WorldModule::OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    m_logic_->Transport(sock, msg_id, msg, len);
}

} // namespace proxy::client