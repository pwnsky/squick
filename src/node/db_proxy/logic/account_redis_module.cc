
#include "account_redis_module.h"

bool AccountRedisModule::Start() {
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_redis_ = pm_->FindModule<IRedisModule>();
    m_pCommonRedisModule = pm_->FindModule<ICommonRedisModule>();

    return true;
}

bool AccountRedisModule::Destory() { return true; }

bool AccountRedisModule::Update() { return true; }

bool AccountRedisModule::AfterStart() { return true; }

bool AccountRedisModule::VerifyAccount(const std::string &account, const std::string &strPwd) {
    if (account.empty() || strPwd.empty()) {
        return false;
    }

    std::string strAccountKey = m_pCommonRedisModule->GetAccountCacheKey(account);
    std::shared_ptr<IRedisClient> driver = m_redis_->GetDriverBySuit(account);
    if (driver) {
        std::string strPassword;
        if (driver->HGET(strAccountKey, "Password", strPassword) && strPassword == strPwd) {
            return true;
        }
    }

    return false;
}

bool AccountRedisModule::AddAccount(const std::string &account, const std::string &strPwd) {
    std::string strAccountKey = m_pCommonRedisModule->GetAccountCacheKey(account);
    std::shared_ptr<IRedisClient> driver = m_redis_->GetDriverBySuit(account);
    if (driver) {
        return driver->HSET(strAccountKey, "Password", strPwd);
    }
    return false;
}

bool AccountRedisModule::ExistAccount(const std::string &account) {
    std::string strAccountKey = m_pCommonRedisModule->GetAccountCacheKey(account);
    std::shared_ptr<IRedisClient> driver = m_redis_->GetDriverBySuit(account);
    if (driver) {
        return driver->EXISTS(strAccountKey);
    }

    return false;
}
