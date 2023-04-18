#include "server_module.h"
#include <forward_list>
#include <squick/plugin/kernel/i_event_module.h>
#include <squick/plugin/kernel/scene_module.h>

namespace lobby::server {
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

} // namespace lobby::server