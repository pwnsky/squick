#include "plugin.h"

#include "node_module.h"
namespace proxy::node {
bool NodeModule::Start() {
    INodeModule::Start();

    m_logic_ = pm_->FindModule<logic::ILogicModule>();
    return true;
}

bool ServerModule::AfterStart() {
    Listen();

    
    return true;
}

bool ServerModule::Destory() { return true; }

bool ServerModule::Update() {
    INodeModule::Update();
    return true;
}

void ServerModule::OnClientDisconnect(socket_t sock) { m_logic_->OnClientDisconnect(sock); }
void ServerModule::OnClientConnected(socket_t sock) { m_logic_->OnClientConnected(sock); }

} // namespace proxy::server