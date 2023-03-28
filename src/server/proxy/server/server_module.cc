#include "plugin.h"

#include "server_module.h"
namespace proxy::server {
bool ServerModule::Start() {
    this->pPluginManager->SetAppType(SQUICK_SERVER_TYPES::SQUICK_ST_PROXY);

    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pProxyToWorldModule = pPluginManager->FindModule<client::IWorldModule>();
    m_pSecurityModule = pPluginManager->FindModule<ISecurityModule>();
    m_pThreadPoolModule = pPluginManager->FindModule<IThreadPoolModule>();

    return true;
}

bool ServerModule::AfterStart() {

    // 绑定Call back以及转发去向
    m_pNetModule->AddEventCallBack(this, &ServerModule::OnSocketClientEvent);
    m_pNetModule->ExpandBufferSize(1024 * 1024 * 2);

    SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_PROXY && pPluginManager->GetAppID() == serverID) {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());
                // const std::string& name = m_pElementModule->GetPropertyString(strId, SquickProtocol::Server::ID());
                // const std::string& ip = m_pElementModule->GetPropertyString(strId, SquickProtocol::Server::IP());

                // 绑定端口
                int nRet = m_pNetModule->Startialization(maxConnect, nPort, nCpus);
                if (nRet < 0) {
                    std::ostringstream strLog;
                    strLog << "Cannot init server net, Port = " << nPort;
                    m_pLogModule->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                    NFASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
                    exit(0);
                }
            }
        }
    }

    return true;
}

bool ServerModule::Destory() { return true; }

bool ServerModule::Update() { return true; }


void ServerModule::OnSocketClientEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
        m_logic_->OnClientDisconnect(sockIndex);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
        m_logic_->OnClientDisconnect(sockIndex);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
        m_logic_->OnClientDisconnect(sockIndex);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
        m_logic_->OnClientConnected(sockIndex);
    }
}



} // namespace proxy::server