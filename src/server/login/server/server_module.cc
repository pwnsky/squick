
#include "server_module.h"
#include "plugin.h"

namespace login::server {
bool ServerModule::Start() {
    this->pPluginManager->SetAppType(SQUICK_SERVER_TYPES::SQUICK_ST_LOGIN);

    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
    m_pLoginToMasterModule = pPluginManager->FindModule<client::IMasterModule>();
    m_pThreadPoolModule = pPluginManager->FindModule<IThreadPoolModule>();

    return true;
}

bool ServerModule::Destory() { return true; }

bool ServerModule::BeforeDestory() { return true; }

bool ServerModule::AfterStart() {
    m_pNetModule->AddReceiveCallBack(this, &ServerModule::InvalidMessage);
    m_pNetModule->AddEventCallBack(this, &ServerModule::OnSocketClientEvent);
    m_pNetModule->ExpandBufferSize();

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
                const int nCpus = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());

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

bool ServerModule::Update() { return true; }

void ServerModule::OnClientConnected(const SQUICK_SOCKET nAddress) {
    NetObject *pObject = m_pNetModule->GetNet()->GetNetObject(nAddress);
    if (pObject) {
        Guid xIdent = m_pKernelModule->CreateGUID();
        pObject->SetClientID(xIdent);
        mxClientIdent.AddElement(xIdent, SQUICK_SHARE_PTR<SQUICK_SOCKET>(SQUICK_NEW SQUICK_SOCKET(nAddress)));
    }
}

void ServerModule::OnClientDisconnect(const SQUICK_SOCKET nAddress) {
    NetObject *pObject = m_pNetModule->GetNet()->GetNetObject(nAddress);
    if (pObject) {
        Guid xIdent = pObject->GetClientID();
        mxClientIdent.RemoveElement(xIdent);
    }
}

void ServerModule::OnSelectWorldProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {}

void ServerModule::OnSocketClientEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
        OnClientDisconnect(sockIndex);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
        OnClientDisconnect(sockIndex);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
        OnClientDisconnect(sockIndex);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
        OnClientConnected(sockIndex);
    }
}

void ServerModule::SynWorldToClient(const SQUICK_SOCKET nFD) {
    SquickStruct::AckServerList xData;

    auto servers = m_pLoginToMasterModule->GetWorldServerMap();

    for (auto& iter : servers) {
        auto& server = iter.second;
        SquickStruct::ServerInfo* si = xData.add_info();
        si->set_name(server.server_name());
        si->set_status(server.server_state());
        si->set_server_id(server.server_id());
        si->set_wait_count(0);
    }
}

void ServerModule::InvalidMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) { printf("Net || umsgID=%d\n", msgID); }

} // namespace login::server