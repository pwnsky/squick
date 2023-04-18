
#include <squick/plugin/config/i_class_module.h>

#include "plugin.h"
#include "world_module.h"
namespace proxy::client {
bool WorldModule::Start() {
    BaseStart();
    m_server_ = pm_->FindModule<server::IServerModule>();
    m_logic_ = pm_->FindModule<logic::ILogicModule>();
    return true;
}

bool WorldModule::Destory() { return true; }

bool WorldModule::Update() {
    BaseUpdate();
    return true;
}

bool WorldModule::AfterStart() {
    m_net_client_->AddReceiveCallBack(ServerType::ST_WORLD, this, &WorldModule::OnOtherMessage);
    AddServer(ServerType::ST_WORLD);
    return true;
}

void WorldModule::LogServerInfo(const std::string &strServerInfo) { m_log_->LogInfo(Guid(), strServerInfo, ""); }

void WorldModule::OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    m_logic_->ForwardToClient(sock, msg_id, msg, len);
}

} // namespace proxy::client