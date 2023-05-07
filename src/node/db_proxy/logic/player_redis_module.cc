
#include "player_redis_module.h"
#include "common_redis_module.h"

PlayerRedisModule::PlayerRedisModule(IPluginManager *p) { pm_ = p; }

bool PlayerRedisModule::Start() {
    m_class_ = pm_->FindModule<IClassModule>();
    m_redis_ = pm_->FindModule<IRedisModule>();
    m_pCommonRedisModule = pm_->FindModule<ICommonRedisModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_pAccountRedisModule = pm_->FindModule<IAccountRedisModule>();

    return true;
}

bool PlayerRedisModule::Destory() { return true; }

bool PlayerRedisModule::Update() { return true; }

bool PlayerRedisModule::AfterStart() { return true; }

bool PlayerRedisModule::LoadPlayerData(const Guid &self, rpc::DbPlayerData &playerData) {
    CommonRedisModule *pCommonRedisModule = (CommonRedisModule *)(m_pCommonRedisModule);
    std::shared_ptr<IPropertyManager> xPropertyManager = m_pCommonRedisModule->GetPropertyInfo(self.ToString(), excel::Player::ThisName(), false, true);
    if (xPropertyManager) {
        *(playerData.mutable_property()->mutable_player_id()) = INetModule::StructToProtobuf(self);
        pCommonRedisModule->ConvertPropertyManagerToPB(xPropertyManager, playerData.mutable_property(), false, true);
        pCommonRedisModule->GetRecordInfo(self.ToString(), excel::Player::ThisName(), playerData.mutable_record(), false, true);
        return true;
    }

    m_log_->LogError(self, "loaded data false", __FUNCTION__, __LINE__);

    return false;
}

bool PlayerRedisModule::SavePlayerData(const Guid &self, const rpc::DbPlayerData &playerData) {
    CommonRedisModule *pCommonRedisModule = (CommonRedisModule *)m_pCommonRedisModule;

    std::shared_ptr<IPropertyManager> xPropManager = pCommonRedisModule->NewPropertyManager(excel::Player::ThisName());
    if (pCommonRedisModule->ConvertPBToPropertyManager(playerData.property(), xPropManager)) {
        m_pCommonRedisModule->SavePropertyInfo(self.ToString(), xPropManager, false, true);
    }

    pCommonRedisModule->SaveRecordInfo(self.ToString(), playerData.record(), -1);

    return true;
}

std::string PlayerRedisModule::GetOnlineGameServerKey() {
    // if (strValue == "**nonexistent-key**")
    return "OnlineGameKey";
}

std::string PlayerRedisModule::GetOnlineProxyServerKey() { return "OnlineProxyKey"; }

bool PlayerRedisModule::ExistRoleName(const std::string &strRoleName) {
    std::shared_ptr<IRedisClient> driver = m_redis_->GetDriverBySuitConsistent();
    if (driver) {
        return driver->EXISTS(strRoleName);
    }

    return false;
}

bool PlayerRedisModule::CreateRole(const std::string &account, const std::string &strRoleName, const Guid &id, const int nHomeSceneID) {
    const std::string strAccountKey = m_pCommonRedisModule->GetAccountCacheKey(account);
    std::shared_ptr<IRedisClient> driver = m_redis_->GetDriverBySuit(account);
    if (driver) {
#ifdef SQUICK_DEV
        std::cout << "在数据库中创建角色 account: " << account << " role_name: " << strRoleName << "\n";
#endif
        if (!driver->EXISTS(strAccountKey)) {
            m_pAccountRedisModule->AddAccount(account, account);
#ifdef SQUICK_DEV
            std::cout << "不存在该账号，创建账号\n";
#endif
        }

        if (driver->EXISTS(strRoleName)) {
#ifdef SQUICK_DEV
            std::cout << "已存在该角色名称" << strRoleName << "\n";
#endif
        }

        if (!driver->EXISTS(strAccountKey)) {
#ifdef SQUICK_DEV
            std::cout << "不存在该账号\n";
#endif
        }

        if (driver->EXISTS(strAccountKey) && !driver->EXISTS(strRoleName)) {
#ifdef SQUICK_DEV
            std::cout << "正在创建角色\n";
#endif
            std::vector<std::string> vecFields;
            std::vector<std::string> vecValues;

            vecFields.push_back(excel::Player::Name());
            vecFields.push_back(excel::Player::ID());

            vecValues.push_back(strRoleName);
            vecValues.push_back(id.ToString());

            driver->HMSET(strAccountKey, vecFields, vecValues);

            std::shared_ptr<IRedisClient> xRoleNameNoSqlDriver = m_redis_->GetDriverBySuitConsistent();
            if (xRoleNameNoSqlDriver) {
                // the name ref to the guid
                xRoleNameNoSqlDriver->HSET(strRoleName, excel::Player::ID(), id.ToString());
            }

            std::shared_ptr<IPropertyManager> xPropertyManager = m_pCommonRedisModule->NewPropertyManager(excel::Player::ThisName());
            if (xPropertyManager) {
                /*
                std::shared_ptr<IProperty> xProperty = xPropertyManager->GetElement(excel::Player::Account());
                if (xProperty)
                {
                        xProperty->SetString(account);
                }


                xProperty = xPropertyManager->GetElement(excel::Player::Name());
                if (xProperty)
                {
                        xProperty->SetString(strRoleName);
                }

                m_pCommonRedisModule->SavePropertyInfo(id.ToString(), xPropertyManager, false, true);
                */
            }
#ifdef SQUICK_DEV
            std::cout << "创建成功\n";
#endif
            return true;
        }
    }
#ifdef SQUICK_DEV
    std::cout << "创建失败\n";
#endif

    return false;
}

bool PlayerRedisModule::GetRoleInfo(const std::string &account, std::string &strRoleName, Guid &id) {
    std::string strAccountKey = m_pCommonRedisModule->GetAccountCacheKey(account);
    std::shared_ptr<IRedisClient> driver = m_redis_->GetDriverBySuit(account);
    if (driver) {
        if (driver->EXISTS(strAccountKey)) {
            std::string strID;
            bool bRoleNameRet = driver->HGET(strAccountKey, excel::Player::Name(), strRoleName);
            bool bRoleIDRet = driver->HGET(strAccountKey, excel::Player::ID(), strID);
            if (bRoleNameRet && bRoleIDRet && !strRoleName.empty() && !strID.empty()) {
                return id.FromString(strID);
            }

            return false;
        }
    }

    return false;
}
