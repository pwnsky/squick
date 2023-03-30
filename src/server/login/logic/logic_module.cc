
#include "logic_module.h"
#include "plugin.h"

namespace login::logic {
bool LogicModule::Start() {
    m_net_ = pm_->FindModule<INetModule>();
    m_log_ = pm_->FindModule<ILogModule>();

    return true;
}

bool LogicModule::Destory() { return true; }

void LogicModule::OnLoginProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
}

bool LogicModule::ReadyUpdate() {
    return true;
}

bool LogicModule::Update() { return true; }

bool LogicModule::AfterStart() { return true; }

} // namespace login::logic