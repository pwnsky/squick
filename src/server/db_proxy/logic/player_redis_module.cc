
#include "player_redis_module.h"
#include "common_redis_module.h"

PlayerRedisModule::PlayerRedisModule(IPluginManager *p) { pPluginManager = p; }

bool PlayerRedisModule::Start() {
    m_pLogicClassModule = pPluginManager->FindModule<IClassModule>();
    m_pNoSqlModule = pPluginManager->FindModule<INoSqlModule>();
    m_pCommonRedisModule = pPluginManager->FindModule<ICommonRedisModule>();
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    m_pAccountRedisModule = pPluginManager->FindModule<IAccountRedisModule>();

    return true;
}

bool PlayerRedisModule::Destory() { return true; }

bool PlayerRedisModule::Update() { return true; }

bool PlayerRedisModule::AfterStart() { return true; }

bool PlayerRedisModule::LoadPlayerData(const Guid &self, SquickStruct::PlayerData &playerData) {
    CommonRedisModule *pCommonRedisModule = (CommonRedisModule *)(m_pCommonRedisModule);
    SQUICK_SHARE_PTR<IPropertyManager> xPropertyManager = m_pCommonRedisModule->GetPropertyInfo(self.ToString(), excel::Player::ThisName(), false, true);
    if (xPropertyManager) {
        *(playerData.mutable_property()->mutable_player_id()) = INetModule::StructToProtobuf(self);
        pCommonRedisModule->ConvertPropertyManagerToPB(xPropertyManager, playerData.mutable_property(), false, true);
        pCommonRedisModule->GetRecordInfo(self.ToString(), excel::Player::ThisName(), playerData.mutable_record(), false, true);
        return true;
    }

    m_pLogModule->LogError(self, "loaded data false", __FUNCTION__, __LINE__);

    return false;
}

bool PlayerRedisModule::SavePlayerData(const Guid &self, const SquickStruct::PlayerData &playerData) {
    CommonRedisModule *pCommonRedisModule = (CommonRedisModule *)m_pCommonRedisModule;

    SQUICK_SHARE_PTR<IPropertyManager> xPropManager = pCommonRedisModule->NewPropertyManager(excel::Player::ThisName());
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
    SQUICK_SHARE_PTR<IRedisClient> xNoSqlDriver = m_pNoSqlModule->GetDriverBySuitConsistent();
    if (xNoSqlDriver) {
        return xNoSqlDriver->EXISTS(strRoleName);
    }

    return false;
}

bool PlayerRedisModule::CreateRole(const std::string &account, const std::string &strRoleName, const Guid &id, const int nHomeSceneID) {
    const std::string strAccountKey = m_pCommonRedisModule->GetAccountCacheKey(account);
    SQUICK_SHARE_PTR<IRedisClient> xNoSqlDriver = m_pNoSqlModule->GetDriverBySuit(account);
    if (xNoSqlDriver) {
#ifdef SQUICK_DEV
        std::cout << "在数据库中创建角色 account: " << account << " role_name: " << strRoleName << "\n";
#endif
        if (!xNoSqlDriver->EXISTS(strAccountKey)) {
            m_pAccountRedisModule->AddAccount(account, account);
#ifdef SQUICK_DEV
            std::cout << "不存在该账号，创建账号\n";
#endif
        }

        if (xNoSqlDriver->EXISTS(strRoleName)) {
#ifdef SQUICK_DEV
            std::cout << "已存在该角色名称" << strRoleName << "\n";
#endif
        }

        if (!xNoSqlDriver->EXISTS(strAccountKey)) {
#ifdef SQUICK_DEV
            std::cout << "不存在该账号\n";
#endif
        }

        if (xNoSqlDriver->EXISTS(strAccountKey) && !xNoSqlDriver->EXISTS(strRoleName)) {
#ifdef SQUICK_DEV
            std::cout << "正在创建角色\n";
#endif
            std::vector<std::string> vecFields;
            std::vector<std::string> vecValues;

            vecFields.push_back(excel::Player::Name());
            vecFields.push_back(excel::Player::ID());

            vecValues.push_back(strRoleName);
            vecValues.push_back(id.ToString());

            xNoSqlDriver->HMSET(strAccountKey, vecFields, vecValues);

            SQUICK_SHARE_PTR<IRedisClient> xRoleNameNoSqlDriver = m_pNoSqlModule->GetDriverBySuitConsistent();
            if (xRoleNameNoSqlDriver) {
                // the name ref to the guid
                xRoleNameNoSqlDriver->HSET(strRoleName, excel::Player::ID(), id.ToString());
            }

            SQUICK_SHARE_PTR<IPropertyManager> xPropertyManager = m_pCommonRedisModule->NewPropertyManager(excel::Player::ThisName());
            if (xPropertyManager) {
                /*
                SQUICK_SHARE_PTR<IProperty> xProperty = xPropertyManager->GetElement(excel::Player::Account());
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
    SQUICK_SHARE_PTR<IRedisClient> xNoSqlDriver = m_pNoSqlModule->GetDriverBySuit(account);
    if (xNoSqlDriver) {
        if (xNoSqlDriver->EXISTS(strAccountKey)) {
            std::string strID;
            bool bRoleNameRet = xNoSqlDriver->HGET(strAccountKey, excel::Player::Name(), strRoleName);
            bool bRoleIDRet = xNoSqlDriver->HGET(strAccountKey, excel::Player::ID(), strID);
            if (bRoleNameRet && bRoleIDRet && !strRoleName.empty() && !strID.empty()) {
                return id.FromString(strID);
            }

            return false;
        }
    }

    return false;
}
