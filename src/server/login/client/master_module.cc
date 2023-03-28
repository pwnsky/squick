
#include "master_module.h"
#include "plugin.h"
#include <squick/struct/struct.h>
namespace login::client {

bool MasterModule::Start() {
    m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pLoginNet_ServerModule = pPluginManager->FindModule<server::IServerModule>();

    return true;
}

bool MasterModule::Destory() { return true; }

bool MasterModule::AfterStart() {
    // m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_MASTER, SquickStruct::ACK_CONNECT_WORLD, this,
    // &LoginToMasterModule::OnSelectServerResultProcess);
    m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_MASTER, SquickStruct::STS_NET_INFO, this, &MasterModule::OnWorldInfoProcess);

    m_pNetClientModule->AddEventCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_MASTER, this, &MasterModule::OnSocketMSEvent);

    m_pNetClientModule->ExpandBufferSize();

    SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_MASTER) {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                // const int maxConnect = m_pElementModule->GetPropertyInt32(strId, SquickProtocol::Server::MaxOnline());
                // const int nCpus = m_pElementModule->GetPropertyInt32(strId, SquickProtocol::Server::CpuCount());
                const std::string &name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

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

bool MasterModule::BeforeDestory() { return false; }

bool MasterModule::Update() {
    ServerReport();
    return true;
}

void MasterModule::Register(INet *pNet) {
    SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_LOGIN && pPluginManager->GetAppID() == serverID) {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
                // const int nCpus = m_pElementModule->GetPropertyInt32(strId, SquickProtocol::Server::CpuCount());
                const std::string &name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

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

                SQUICK_SHARE_PTR<ConnectData> pServerData = m_pNetClientModule->GetServerNetInfo(pNet);
                if (pServerData) {
                    int nTargetID = pServerData->nGameID;
                    m_pNetClientModule->SendToServerByPB(nTargetID, SquickStruct::ServerRPC::LOGIN_TO_MASTER_REGISTERED, xMsg);
                    m_pLogModule->LogInfo(Guid(0, pData->server_id()), pData->server_name(), "Register");
                }
            }
        }
    }
}

void MasterModule::ServerReport() {
    if (mLastReportTime + 10 > pPluginManager->GetNowTime()) {
        return;
    }
    mLastReportTime = pPluginManager->GetNowTime();
    std::shared_ptr<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
            if (pPluginManager->GetAppID() == serverID) {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const std::string &name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

                SquickStruct::ServerInfoReport reqMsg;

                reqMsg.set_server_id(serverID);
                reqMsg.set_server_name(strId);
                reqMsg.set_server_cur_count(0);
                reqMsg.set_server_ip(ip);
                reqMsg.set_server_port(nPort);
                reqMsg.set_server_max_online(maxConnect);
                reqMsg.set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
                reqMsg.set_server_type(serverType);

                m_pNetClientModule->SendToAllServerByPB(SQUICK_SERVER_TYPES::SQUICK_ST_MASTER, SquickStruct::STS_SERVER_REPORT, reqMsg, Guid());
            }
        }
    }
}

void MasterModule::OnSelectServerResultProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {}

void MasterModule::OnSocketMSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
        Register(pNet);
    }
}

void MasterModule::OnWorldInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList req;
    if (!INetModule::ReceivePB(msgID, msg, len, req, nPlayerID)) {
        return;
    }

    for (int i = 0; i < req.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &si = req.server_list(i);

            dout << "登录服务器收到服务列表: " << si.server_id() << "   " << si.server_type() << std::endl;
            if (si.server_type() == SQUICK_SERVER_TYPES::SQUICK_ST_WORLD) {
                world_servers_[si.server_id()] = si;
            } else if(si.server_type() == SQUICK_SERVER_TYPES::SQUICK_ST_PROXY){
                proxy_servers_[si.server_id()] = si;
            }
    }
    m_pLogModule->LogInfo(Guid(0, req.server_list_size()), "", "WorldInfo");
}

INetClientModule *MasterModule::GetClusterModule() { return m_pNetClientModule; }

map<int, SquickStruct::ServerInfoReport> &MasterModule::GetWorldServers() { return world_servers_; }

map<int, SquickStruct::ServerInfoReport>& MasterModule::GetProxyServers() { return proxy_servers_; }

} // namespace login::client