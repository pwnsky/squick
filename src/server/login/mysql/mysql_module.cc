
#include "mysql_module.h"
#include "plugin.h"

namespace login::mysql {
bool MysqlModule::Start() {
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();

    return true;
}

bool MysqlModule::Destory() { return true; }

void MysqlModule::OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
}

bool MysqlModule::ReadyUpdate() {
    return true;
}

bool MysqlModule::Update() { return true; }

bool MysqlModule::AfterStart() { return true; }

} // namespace login::logic