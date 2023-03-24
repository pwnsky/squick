
#include "redis_module.h"
#include "plugin.h"

namespace login::redis {
bool RedisModule::Start() {
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();

    return true;
}

bool RedisModule::Destory() { return true; }

bool RedisModule::GenerateProxyConnectKey(const Guid& guid, int proxy_id, const string& key) {
    return true;
}

bool RedisModule::ReadyUpdate() {
    return true;
}

bool RedisModule::Update() { return true; }

bool RedisModule::AfterStart() { return true; }

} // namespace login::logic