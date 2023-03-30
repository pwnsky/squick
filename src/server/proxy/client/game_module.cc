
#include "game_module.h"
#include "plugin.h"
#include <squick/plugin/config/i_class_module.h>
namespace proxy::client {
bool GameModule::Start() {
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_logic_ = pm_->FindModule<logic::ILogicModule>();
    return true;
}

bool GameModule::Destory() {
    // Final();
    // Clear();
    return true;
}

bool GameModule::Update() { return true; }

bool GameModule::AfterStart() {
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_GAME, SquickStruct::GameLobbyRPC::ACK_ENTER, this, &GameModule::OnAckEnterGame);
    m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_GAME, this, &GameModule::Transport);

    m_net_client_->AddEventCallBack(ServerType::SQUICK_ST_GAME, this, &GameModule::OnSocketGSEvent);
    m_net_client_->ExpandBufferSize();

    return true;
}

void GameModule::OnSocketGSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
        Register(pNet);
    }
}

void GameModule::Register(INet *pNet) {
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
                    m_net_client_->SendToServerByPB(nTargetID, SquickStruct::ServerRPC::PROXY_TO_GAME_REGISTERED, xMsg);

                    m_log_->LogInfo(Guid(0, pData->server_id()), pData->server_name(), "Register");
                }
            }
        }
    }
}
/**
 * 进入游戏通知玩家
 */
void GameModule::OnAckEnterGame(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "进入游戏成功!\n";
    Guid nPlayerID;
    SquickStruct::AckEnter xData;
    if (!INetModule::ReceivePB(msg_id, msg, len, xData, nPlayerID)) {
        return;
    }

    const Guid &xClient = INetModule::ProtobufToStruct(xData.client());
    const Guid &xPlayer = INetModule::ProtobufToStruct(xData.object());

    //m_logic_->EnterGameSuccessEvent(xClient, xPlayer);
    m_logic_->Transport(sock, msg_id, msg, len);
}

void GameModule::LogServerInfo(const std::string &strServerInfo) { m_log_->LogInfo(Guid(), strServerInfo, ""); }

void GameModule::Transport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    m_logic_->Transport(sock, msg_id, msg, len);
}

} // namespace proxy::client