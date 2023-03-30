#include "plugin.h"

#include "server_module.h"
namespace proxy::server {
bool ServerModule::Start() {
    this->pm_->SetAppType(ServerType::SQUICK_ST_PROXY);

    m_net_ = pm_->FindModule<INetModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_pProxyToWorldModule = pm_->FindModule<client::IWorldModule>();
    m_security_ = pm_->FindModule<ISecurityModule>();
    m_pThreadPoolModule = pm_->FindModule<IThreadPoolModule>();
    m_logic_ = pm_->FindModule<logic::ILogicModule>();
    return true;
}

bool ServerModule::AfterStart() {

    // 绑定Call back以及转发去向
    m_net_->AddEventCallBack(this, &ServerModule::OnSocketClientEvent);
    m_net_->ExpandBufferSize(1024 * 1024 * 2);

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::SQUICK_ST_PROXY && pm_->GetAppID() == serverID) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_element_->GetPropertyInt32(strId, excel::Server::CpuCount());
                // const std::string& name = m_element_->GetPropertyString(strId, SquickProtocol::Server::ID());
                // const std::string& ip = m_element_->GetPropertyString(strId, SquickProtocol::Server::IP());

                // 绑定端口
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

bool ServerModule::Destory() { return true; }

bool ServerModule::Update() { return true; }


void ServerModule::OnSocketClientEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
        m_logic_->OnClientDisconnect(sock);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
        m_logic_->OnClientDisconnect(sock);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
        m_logic_->OnClientDisconnect(sock);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
        m_logic_->OnClientConnected(sock);
    }
}



} // namespace proxy::server