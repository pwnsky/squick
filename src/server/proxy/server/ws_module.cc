
#include "ws_module.h"
#include "plugin.h"
#include <squick/plugin/kernel/i_kernel_module.h>
namespace proxy::server {
bool ProxyServerNet_WSModule::Start() {
    m_pWSModule = pm_->FindModule<IWSModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_pProxyToWorldModule = pm_->FindModule<client::IWorldModule>();
    m_security_ = pm_->FindModule<ISecurityModule>();

    return true;
}

bool ProxyServerNet_WSModule::AfterStart() {
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::ST_PROXY && pm_->GetAppID() == serverID) {
                const int port = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int cpuCount = m_element_->GetPropertyInt32(strId, excel::Server::CpuCount());
                // const std::string& name = m_element_->GetPropertyString(strId, SquickProtocol::Server::ID());
                // const std::string& ip = m_element_->GetPropertyString(strId, SquickProtocol::Server::IP());
                int wsPort = m_element_->GetPropertyInt32(strId, excel::Server::WSPort());

                // web server only run one instance in each server
                if (wsPort > 0) {
                    int nRet = m_pWSModule->Startialization(maxConnect, wsPort, cpuCount);
                    if (nRet < 0) {
                        std::ostringstream strLog;
                        strLog << "Cannot init websocket server net, Port = " << wsPort;
                        m_log_->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                        SQUICK_ASSERT(nRet, "Cannot init websocket server net", __FILE__, __FUNCTION__);
                        exit(0);
                    }

                    m_pWSModule->AddEventCallBack(this, &ProxyServerNet_WSModule::OnSocketClientEvent);
                    m_pWSModule->AddReceiveCallBack(this, &ProxyServerNet_WSModule::OnWebSocketTestProcess);
                    break;
                }
            }
        }
    }

    return true;
}

bool ProxyServerNet_WSModule::Destory() { return true; }

bool ProxyServerNet_WSModule::Update() { return true; }

void ProxyServerNet_WSModule::OnWebSocketTestProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    std::cout << "Simple WebSocket Test Chat\n";
    m_pWSModule->SendMsgToAllClient(std::string(msg, len));
}

void ProxyServerNet_WSModule::OnSocketClientEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
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

void ProxyServerNet_WSModule::OnClientDisconnect(const socket_t sock) {
    NetObject *pNetObject = m_pWSModule->GetNet()->GetNetObject(sock);
    if (pNetObject) {
        int nGameID = pNetObject->GetGameID();
        if (nGameID > 0) {
            // when a net-object bind a account then tell that game-server
            if (!pNetObject->GetUserID().IsNull()) {
                /*
                SquickStruct::ReqLeaveGameServer xData;
                xData.set_arg(nGameID);

                SquickStruct::MsgBase xMsg;

                //real user id
                *xMsg.mutable_player_id() = INetModule::StructToProtobuf(pNetObject->GetUserID());

                if (!xData.SerializeToString(xMsg.mutable_msg_data()))
                {
                    return;
                }

                std::string msg;
                if (!xMsg.SerializeToString(&msg))
                {
                    return;
                }

                m_net_client_->SendByServerIDWithOutHead(nGameID, SquickStruct::EGameMsgID::REQ_LEAVE_GAME, msg);
                 */
            }
        }

        mxClientIdent.RemoveElement(pNetObject->GetClientID());
    }
}

void ProxyServerNet_WSModule::OnClientConnected(const socket_t sock) {
    // bind client'id with socket id
    NetObject *pNetObject = m_pWSModule->GetNet()->GetNetObject(sock);
    if (pNetObject && pNetObject->GetClientID().IsNull()) {
        Guid xClientIdent = m_kernel_->CreateGUID();
        pNetObject->SetClientID(xClientIdent);
        mxClientIdent.AddElement(xClientIdent, std::shared_ptr<socket_t>(new socket_t(sock)));

        // 1. create a tcp client to connect to the TCP service provided by proxy server.
        // 2. transfer the ws data come from websocket to the TCP service provided by proxy server.
        // 3.transfer the tcp data come from proxy server to the websocket service to send to clients.
    }
}

} // namespace proxy::server