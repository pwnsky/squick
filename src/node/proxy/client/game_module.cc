
#include "game_module.h"
#include "plugin.h"
#include <squick/plugin/config/i_class_module.h>
namespace proxy::client {
bool GameModule::Start() {
    BaseStart();
    return true;
}

bool GameModule::Destory() { return true; }

bool GameModule::Update() {
    BaseUpdate();
    return true;
}

bool GameModule::AfterStart() {
    m_net_client_->AddReceiveCallBack(ServerType::ST_GAME, rpc::LobbyBaseRPC::ACK_ENTER, this, &GameModule::OnAckEnterGame);
    m_net_client_->AddReceiveCallBack(ServerType::ST_GAME, this, &GameModule::Transport);
    AddServer(ServerType::ST_GAME);
    return true;
}

/**
 * 进入游戏通知玩家
 */
void GameModule::OnAckEnterGame(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "进入游戏成功!\n";
    Guid nPlayerID;
    rpc::AckEnter xData;
    if (!INetModule::ReceivePB(msg_id, msg, len, xData, nPlayerID)) {
        return;
    }
    const Guid &xClient = INetModule::ProtobufToStruct(xData.guid());
    const Guid &xPlayer = INetModule::ProtobufToStruct(xData.object());

    // m_logic_->EnterGameSuccessEvent(xClient, xPlayer);
    m_logic_->ForwardToClient(sock, msg_id, msg, len);
}

void GameModule::LogServerInfo(const std::string &strServerInfo) { m_log_->LogInfo(Guid(), strServerInfo, ""); }

void GameModule::Transport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { m_logic_->ForwardToClient(sock, msg_id, msg, len); }

} // namespace proxy::client