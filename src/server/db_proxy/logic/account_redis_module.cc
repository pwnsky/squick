
#include "account_redis_module.h"

bool AccountRedisModule::Start() {
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pLogicClassModule = pPluginManager->FindModule<IClassModule>();
    m_pNoSqlModule = pPluginManager->FindModule<INoSqlModule>();
    m_pCommonRedisModule = pPluginManager->FindModule<ICommonRedisModule>();

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
    SQUICK_SHARE_PTR<IRedisClient> xNoSqlDriver = m_pNoSqlModule->GetDriverBySuit(account);
    if (xNoSqlDriver) {
        std::string strPassword;
        if (xNoSqlDriver->HGET(strAccountKey, "Password", strPassword) && strPassword == strPwd) {
            return true;
        }
    }

    return false;
}

bool AccountRedisModule::AddAccount(const std::string &account, const std::string &strPwd) {
    std::string strAccountKey = m_pCommonRedisModule->GetAccountCacheKey(account);
    SQUICK_SHARE_PTR<IRedisClient> xNoSqlDriver = m_pNoSqlModule->GetDriverBySuit(account);
    if (xNoSqlDriver) {
        return xNoSqlDriver->HSET(strAccountKey, "Password", strPwd);
    }
    return false;
}

bool AccountRedisModule::ExistAccount(const std::string &account) {
    std::string strAccountKey = m_pCommonRedisModule->GetAccountCacheKey(account);
    SQUICK_SHARE_PTR<IRedisClient> xNoSqlDriver = m_pNoSqlModule->GetDriverBySuit(account);
    if (xNoSqlDriver) {
        return xNoSqlDriver->EXISTS(strAccountKey);
    }

    return false;
}
