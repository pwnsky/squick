
#include "game_module.h"
#include "plugin.h"
#include <squick/plugin/config/i_class_module.h>
#include <squick/struct/struct.h>
namespace gameplay_manager::client {
bool GameModule::Start() { return true; }

bool GameModule::Destory() {
    // Final();
    // Clear();
    return true;
}

bool GameModule::Update() { return true; }

bool GameModule::AfterStart() {
    dout << " Game Modull bind\n";
    m_net_client_->AddReceiveCallBack(ServerType::ST_GAME, this, &GameModule::Transport);

    return true;
}

void GameModule::Transport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "PvpManager转发消息给PVP, msg_id: " << msg_id << std::endl;
    m_ServerModule->Transport(sock, msg_id, msg, len);
}

} // namespace gameplay_manager::client