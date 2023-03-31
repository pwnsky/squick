#include "plugin.h"
#include "login_module.h"


namespace proxy::client {
bool LoginModule::Start() {
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    return true;
}


bool LoginModule::AfterStart() {
    m_net_client_->AddReceiveCallBack(ServerType::ST_LOGIN, rpc::LoginRPC::REQ_PROXY_CONNECT_VERIFY, this, &LoginModule::OnAckProxyConnectVerify);
    m_net_client_->AddEventCallBack(ServerType::ST_LOGIN, this, &LoginModule::OnSocketLSEvent);
    m_net_client_->ExpandBufferSize();


    // 连接登录服务器
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string>& strIdList = xLogicClass->GetIDList();

        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string& strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            const int nServerArea = m_element_->GetPropertyInt32(strId, excel::Server::Area());
            if (serverType == ServerType::ST_LOGIN) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const std::string& name = m_element_->GetPropertyString(strId, excel::Server::ID());
                const std::string& ip = m_element_->GetPropertyString(strId, excel::Server::IP());

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

bool LoginModule::Destory() {
    return true;
}

bool LoginModule::Update() {

    return true;
}

bool LoginModule::OnReqProxyConnectVerify(const std::string &guid, const std::string &key) {
    return true;
}

void LoginModule::OnAckProxyConnectVerify(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    return ;
}

void LoginModule::OnSocketLSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
    }
}




} // namespace proxy::client