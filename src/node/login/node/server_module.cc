
#include "server_module.h"
#include "plugin.h"

namespace login::server {
bool ServerModule::Start() {
    BaseStart();
    return true;
}

bool ServerModule::Destory() { return true; }

bool ServerModule::BeforeDestory() { return true; }

bool ServerModule::AfterStart() {
    m_net_->AddReceiveCallBack(this, &ServerModule::InvalidMessage);
    Listen();
    return true;
}

bool ServerModule::Update() { return true; }

void ServerModule::OnClientConnected(const socket_t sock) {}

void ServerModule::OnClientDisconnect(const socket_t sock) {}

void ServerModule::InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { printf("Net || umsg_id=%d\n", msg_id); }

} // namespace login::server