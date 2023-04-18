
#include "redis_module.h"
#include "plugin.h"

namespace login::redis {
bool RedisModule::Start() {
    m_net_ = pm_->FindModule<INetModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_redis_ = pm_->FindModule<::IRedisModule>();

    return true;
}

bool RedisModule::Destory() { return true; }

bool RedisModule::GenerateProxyConnectKey(const Guid &guid, int proxy_id, const string &key) { return true; }

bool RedisModule::ReadyUpdate() { return true; }

bool RedisModule::Update() { return true; }

bool RedisModule::AfterStart() { return true; }

bool RedisModule::HashSet(const std::string &guid, const std::string &key, const std::string &value) {
    std::shared_ptr<IRedisClient> driver = m_redis_->GetDriverBySuit(guid);
    if (driver) {
        string guid_key = "login_" + guid;
        driver->HSET(guid_key, key, value);
        return true;
    }
    return false;
}

bool RedisModule::HashGet(const std::string &guid, const std::string &key, std::string &value) {
    std::shared_ptr<IRedisClient> driver = m_redis_->GetDriverBySuit(guid);
    if (driver) {
        string guid_key = "login_" + guid;
        driver->HGET(guid_key, key, value);
        return true;
    }
    return false;
}

} // namespace login::redis
