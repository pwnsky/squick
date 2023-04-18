
#include "server_module.h"
#include "plugin.h"
#include <squick/struct/struct.h>
namespace world::server {
bool ServerModule::Start() {
    BaseStart();
    return true;
}

bool ServerModule::AfterStart() {
    Listen();
    return true;
}

bool ServerModule::Destory() { return true; }

bool ServerModule::Update() { return true; }

void ServerModule::OnClientDisconnect(const socket_t sock) {}

void ServerModule::OnClientConnected(const socket_t sock) {}

void ServerModule::OnServerReport(const socket_t nFd, const int msg_id, const char *buffer, const uint32_t len) {
    Guid xGUID;
    rpc::Server msg;
    if (!m_net_->ReceivePB(msg_id, buffer, len, msg, xGUID)) {
        return;
    }
    m_net_client_->SendToAllServerByPB(ServerType::ST_MASTER, rpc::SERVER_REPORT, msg, Guid());
}

} // namespace world::server