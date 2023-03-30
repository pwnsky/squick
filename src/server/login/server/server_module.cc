
#include "server_module.h"
#include "plugin.h"

namespace login::server {
bool ServerModule::Start() {
    this->pm_->SetAppType(ServerType::ST_LOGIN);

    m_net_ = pm_->FindModule<INetModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_pLoginToMasterModule = pm_->FindModule<client::IMasterModule>();
    m_pThreadPoolModule = pm_->FindModule<IThreadPoolModule>();

    return true;
}

bool ServerModule::Destory() { return true; }

bool ServerModule::BeforeDestory() { return true; }

bool ServerModule::AfterStart() {
    m_net_->AddReceiveCallBack(this, &ServerModule::InvalidMessage);
    m_net_->AddEventCallBack(this, &ServerModule::OnSocketClientEvent);
    m_net_->ExpandBufferSize();

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
                const int nCpus = m_element_->GetPropertyInt32(strId, excel::Server::CpuCount());

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

    return true;
}

bool ServerModule::Update() { return true; }

void ServerModule::OnClientConnected(const socket_t sock) {
    NetObject *pObject = m_net_->GetNet()->GetNetObject(sock);
    if (pObject) {
        Guid xIdent = m_kernel_->CreateGUID();
        pObject->SetClientID(xIdent);
        mxClientIdent.AddElement(xIdent, std::shared_ptr<socket_t>(new socket_t(sock)));
    }
}

void ServerModule::OnClientDisconnect(const socket_t sock) {
    NetObject *pObject = m_net_->GetNet()->GetNetObject(sock);
    if (pObject) {
        Guid xIdent = pObject->GetClientID();
        mxClientIdent.RemoveElement(xIdent);
    }
}

void ServerModule::OnSelectWorldProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void ServerModule::OnSocketClientEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
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

void ServerModule::SynWorldToClient(const socket_t nFD) {
    SquickStruct::AckServerList xData;

    auto servers = m_pLoginToMasterModule->GetWorldServers();

    for (auto& iter : servers) {
        auto& server = iter.second;
        SquickStruct::ServerInfo* si = xData.add_info();
        si->set_name(server.server_name());
        si->set_status(server.server_state());
        si->set_server_id(server.server_id());
        si->set_wait_count(0);
    }
}

void ServerModule::InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { printf("Net || umsg_id=%d\n", msg_id); }

} // namespace login::server