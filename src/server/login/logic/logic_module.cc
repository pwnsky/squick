
#include "logic_module.h"
#include "plugin.h"

namespace login::logic {
bool LogicModule::Start() {
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();

    return true;
}

bool LogicModule::Destory() { return true; }

void LogicModule::OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
}

bool LogicModule::ReadyUpdate() {
    return true;
}

bool LogicModule::Update() { return true; }

bool LogicModule::AfterStart() { return true; }

} // namespace login::logic