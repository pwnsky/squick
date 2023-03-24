
#include "redis_module.h"
#include "plugin.h"

namespace proxy::redis {
bool RedisModule::Start() {
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();

    return true;
}

bool RedisModule::Destory() { return true; }

void RedisModule::OnLoginProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
}

bool RedisModule::ReadyUpdate() {
    return true;
}

bool RedisModule::Update() { return true; }

bool RedisModule::AfterStart() { return true; }

} // namespace login::logic