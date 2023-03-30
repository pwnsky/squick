
#include "master_module.h"
#include "plugin.h"
#include <squick/struct/struct.h>
namespace login::client {

bool MasterModule::Start() {
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_pLoginNet_ServerModule = pm_->FindModule<server::IServerModule>();

    return true;
}

bool MasterModule::Destory() { return true; }

bool MasterModule::AfterStart() {
    // m_net_client_->AddReceiveCallBack(ServerType::ST_MASTER, SquickStruct::ACK_CONNECT_WORLD, this,
    // &LoginToMasterModule::OnSelectServerResultProcess);
    m_net_client_->AddReceiveCallBack(ServerType::ST_MASTER, SquickStruct::STS_NET_INFO, this, &MasterModule::OnWorldInfoProcess);

    m_net_client_->AddEventCallBack(ServerType::ST_MASTER, this, &MasterModule::OnSocketMSEvent);

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

bool MasterModule::BeforeDestory() { return false; }

bool MasterModule::Update() {
    ServerReport();
    return true;
}

void MasterModule::Register(INet *pNet) {
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::ST_LOGIN && pm_->GetAppID() == serverID) {
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
                    m_net_client_->SendToServerByPB(nTargetID, SquickStruct::ServerRPC::LOGIN_TO_MASTER_REGISTERED, xMsg);
                    m_log_->LogInfo(Guid(0, pData->server_id()), pData->server_name(), "Register");
                }
            }
        }
    }
}

void MasterModule::ServerReport() {
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

                m_net_client_->SendToAllServerByPB(ServerType::ST_MASTER, SquickStruct::STS_SERVER_REPORT, reqMsg, Guid());
            }
        }
    }
}

void MasterModule::OnSelectServerResultProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void MasterModule::OnSocketMSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
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

void MasterModule::OnWorldInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList req;
    if (!INetModule::ReceivePB(msg_id, msg, len, req, nPlayerID)) {
        return;
    }

    for (int i = 0; i < req.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &si = req.server_list(i);

            dout << "登录服务器收到服务列表: " << si.server_id() << "   " << si.server_type() << std::endl;
            if (si.server_type() == ServerType::ST_WORLD) {
                world_servers_[si.server_id()] = si;
            } else if(si.server_type() == ServerType::ST_PROXY){
                proxy_servers_[si.server_id()] = si;
            }
    }
    m_log_->LogInfo(Guid(0, req.server_list_size()), "", "WorldInfo");
}

INetClientModule *MasterModule::GetClusterModule() { return m_net_client_; }

map<int, SquickStruct::ServerInfoReport> &MasterModule::GetWorldServers() { return world_servers_; }

map<int, SquickStruct::ServerInfoReport>& MasterModule::GetProxyServers() { return proxy_servers_; }

} // namespace login::client