

#include "common_redis_module.h"
#include <squick/core/property_manager.h>
#include <squick/core/record_manager.h>

CommonRedisModule::CommonRedisModule(IPluginManager *p) { pm_ = p; }

std::string CommonRedisModule::GetPropertyCacheKey(const std::string &self) { return self + "_ObjectProperty"; }

std::string CommonRedisModule::GetRecordCacheKey(const std::string &self) { return self + "_ObjectRecord"; }

std::string CommonRedisModule::GetFriendCacheKey(const std::string &self) { return self + "_ObjectFriend"; }

std::string CommonRedisModule::GetFriendInviteCacheKey(const std::string &self) { return self + "_ObjectFriendInvite"; }

std::string CommonRedisModule::GetBlockCacheKey(const std::string &self) { return self + "_ObjectBlock"; }

std::string CommonRedisModule::GetTeamCacheKey(const std::string &self) { return self + "_Team"; }

std::string CommonRedisModule::GetTeamInviteCacheKey(const std::string &self) { return self + "_TeamInvite"; }

std::string CommonRedisModule::GetAccountCacheKey(const std::string &account) { return account + "_AccountInfo"; }

std::string CommonRedisModule::GetTileCacheKey(const int &sceneID) { return lexical_cast<std::string>(sceneID) + "_TileInfo"; }

std::string CommonRedisModule::GetSceneCacheKey(const int &sceneID) { return lexical_cast<std::string>(sceneID) + "_SceneProps"; }

std::string CommonRedisModule::GetCellCacheKey(const std::string &strCellID) { return strCellID + "_CellInfo"; }

bool CommonRedisModule::AfterStart() {
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_redis_ = pm_->FindModule<IRedisModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();

    return true;
}

std::shared_ptr<IPropertyManager> CommonRedisModule::NewPropertyManager(const std::string &className) {
    std::shared_ptr<IPropertyManager> pStaticClassPropertyManager = m_class_->GetClassPropertyManager(className);
    if (pStaticClassPropertyManager) {
        Guid ident;
        std::shared_ptr<IPropertyManager> pPropertyManager(new PropertyManager(ident));

        std::shared_ptr<IProperty> pStaticConfigPropertyInfo = pStaticClassPropertyManager->First();
        while (pStaticConfigPropertyInfo) {
            if (pStaticConfigPropertyInfo->GetSave() || pStaticConfigPropertyInfo->GetCache()) {
                std::shared_ptr<IProperty> xProperty =
                    pPropertyManager->AddProperty(ident, pStaticConfigPropertyInfo->GetKey(), pStaticConfigPropertyInfo->GetType());

                xProperty->SetPublic(pStaticConfigPropertyInfo->GetPublic());
                xProperty->SetPrivate(pStaticConfigPropertyInfo->GetPrivate());
                xProperty->SetSave(pStaticConfigPropertyInfo->GetSave());
                xProperty->SetCache(pStaticConfigPropertyInfo->GetCache());
                xProperty->SetRef(pStaticConfigPropertyInfo->GetRef());
            }

            pStaticConfigPropertyInfo = pStaticClassPropertyManager->Next();
        }

        return pPropertyManager;
    }

    return std::shared_ptr<IPropertyManager>(NULL);
}

std::shared_ptr<IRecordManager> CommonRedisModule::NewRecordManager(const std::string &className) {
    std::shared_ptr<IRecordManager> pStaticClassRecordManager = m_class_->GetClassRecordManager(className);
    if (pStaticClassRecordManager) {
        Guid ident;
        std::shared_ptr<IRecordManager> pRecordManager(new RecordManager(ident));

        std::shared_ptr<IRecord> pConfigRecordInfo = pStaticClassRecordManager->First();
        while (pConfigRecordInfo) {
            if (pConfigRecordInfo->GetSave() || pConfigRecordInfo->GetCache()) {
                std::shared_ptr<IRecord> xRecord = pRecordManager->AddRecord(ident, pConfigRecordInfo->GetName(), pConfigRecordInfo->GetStartData(),
                                                                             pConfigRecordInfo->GetTag(), pConfigRecordInfo->GetRows());

                xRecord->SetPublic(pConfigRecordInfo->GetPublic());
                xRecord->SetPrivate(pConfigRecordInfo->GetPrivate());
                xRecord->SetSave(pConfigRecordInfo->GetSave());
                xRecord->SetCache(pConfigRecordInfo->GetCache());
            }

            pConfigRecordInfo = pStaticClassRecordManager->Next();
        }

        return pRecordManager;
    }

    return std::shared_ptr<IRecordManager>(NULL);
}

std::shared_ptr<IPropertyManager> CommonRedisModule::GetPropertyInfo(const std::string &self, const std::string &className,
                                                                     std::vector<std::string> &vKeyCacheList, std::vector<std::string> &vValueCacheList,
                                                                     const bool cache, const bool save, std::shared_ptr<IPropertyManager> propertyManager) {
    // TODO optimize
    std::shared_ptr<IPropertyManager> pPropertyManager = propertyManager;
    if (pPropertyManager == nullptr) {
        pPropertyManager = NewPropertyManager(className);
        if (!pPropertyManager) {
            return nullptr;
        }
    }

    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return nullptr;
    }

    // TODO
    // just run this function one time
    std::shared_ptr<IProperty> xProperty = pPropertyManager->First();
    while (xProperty) {
        if ((cache && xProperty->GetCache()) || (save && xProperty->GetSave())) {
            vKeyCacheList.push_back(xProperty->GetKey());
        }

        xProperty = pPropertyManager->Next();
    }

    // cache
    std::string strCacheKey = GetPropertyCacheKey(self);
    if (!pDriver->HMGET(strCacheKey, vKeyCacheList, vValueCacheList)) {
        return nullptr;
    }

    if (vKeyCacheList.size() == vValueCacheList.size()) {
        ConvertVectorToPropertyManager(vKeyCacheList, vValueCacheList, pPropertyManager, cache, save);

        return pPropertyManager;
    }

    return nullptr;
}

std::shared_ptr<IRecordManager> CommonRedisModule::GetRecordInfo(const std::string &self, const std::string &className, std::vector<std::string> &vKeyCacheList,
                                                                 std::vector<std::string> &vValueCacheList, const bool cache, const bool save,
                                                                 std::shared_ptr<IRecordManager> recordManager) {
    std::shared_ptr<IRecordManager> pRecordManager = recordManager;
    if (pRecordManager == nullptr) {
        pRecordManager = NewRecordManager(className);
        if (!pRecordManager) {
            return nullptr;
        }
    }

    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return nullptr;
    }

    // TODO
    // just run this function one time
    std::shared_ptr<IRecord> xRecord = pRecordManager->First();
    while (xRecord) {
        if ((cache && xRecord->GetCache()) || (save && xRecord->GetSave())) {
            vKeyCacheList.push_back(xRecord->GetName());
        }

        xRecord = pRecordManager->Next();
    }

    // cache
    std::string strCacheKey = GetRecordCacheKey(self);
    if (!pDriver->HMGET(strCacheKey, vKeyCacheList, vValueCacheList)) {
        return nullptr;
    }

    if (vKeyCacheList.size() == vValueCacheList.size()) {
        ConvertVectorToRecordManager(vKeyCacheList, vValueCacheList, pRecordManager, cache, save);
        return pRecordManager;
    }

    return nullptr;
}

std::shared_ptr<IPropertyManager> CommonRedisModule::GetPropertyInfo(const std::string &self, const std::string &className, const bool cache, const bool save,
                                                                     std::shared_ptr<IPropertyManager> propertyManager) {
    std::vector<std::string> vKeyCacheList;
    std::vector<std::string> vValueCacheList;

    return GetPropertyInfo(self, className, vKeyCacheList, vValueCacheList, cache, save, propertyManager);
}

std::shared_ptr<IRecordManager> CommonRedisModule::GetRecordInfo(const std::string &self, const std::string &className, const bool cache, const bool save,
                                                                 std::shared_ptr<IRecordManager> recordManager) {
    std::vector<std::string> vKeyCacheList;
    std::vector<std::string> vValueCacheList;

    return GetRecordInfo(self, className, vKeyCacheList, vValueCacheList, cache, save, recordManager);
}

bool CommonRedisModule::GetRecordInfo(const std::string &self, const std::string &className, rpc::ObjectRecordList *pRecordDataList, const bool cache,
                                      const bool save) {
    *(pRecordDataList->mutable_player_id()) = INetModule::StructToProtobuf(Guid(self));

    std::vector<std::string> vKeyCacheList;
    std::vector<std::string> vValueCacheList;
    std::shared_ptr<IRecordManager> xRecordManager = GetRecordInfo(self, className, vKeyCacheList, vValueCacheList, cache, save);
    if (xRecordManager && vKeyCacheList.size() == vValueCacheList.size()) {
        if (ConvertRecordManagerToPB(xRecordManager, pRecordDataList, cache, save)) {
            return true;
        }
    }

    return false;
}

bool CommonRedisModule::SavePropertyInfo(const std::string &self, std::shared_ptr<IPropertyManager> pPropertyManager, const bool cache, const bool save,
                                         const int nExpireSecond) {
    if (!pPropertyManager) {
        return false;
    }

    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return false;
    }

    std::vector<std::string> vKeyList;
    std::vector<std::string> vValueList;
    if (!ConvertPropertyManagerToVector(pPropertyManager, vKeyList, vValueList, cache, save)) {
        return false;
    }

    if (vKeyList.size() != vValueList.size()) {
        return false;
    }

    std::string strKey = GetPropertyCacheKey(self);

    if (!pDriver->HMSET(strKey, vKeyList, vValueList)) {
        return false;
    }

    if (nExpireSecond > 0) {
        pDriver->EXPIRE(strKey, nExpireSecond);
    }

    return true;
}

bool CommonRedisModule::SaveRecordInfo(const std::string &self, std::shared_ptr<IRecordManager> pRecordManager, const bool cache, const bool save,
                                       const int nExpireSecond) {
    if (!pRecordManager) {
        return false;
    }

    rpc::ObjectRecordList xRecordList;
    if (ConvertRecordManagerToPB(pRecordManager, &xRecordList, cache, save)) {
        return SaveRecordInfo(self, xRecordList, nExpireSecond);
    }

    return false;
}

bool CommonRedisModule::SaveRecordInfo(const std::string &self, const rpc::ObjectRecordList &xRecordData, const int nExpireSecond) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return false;
    }

    std::vector<std::string> vKeyList;
    std::vector<std::string> vValueList;

    for (int i = 0; i < xRecordData.record_list_size(); ++i) {
        const rpc::ObjectRecordBase &xRecord = xRecordData.record_list(i);

        std::string strValue;
        if (!xRecord.SerializeToString(&strValue)) {
            continue;
        }

        vKeyList.push_back(xRecord.record_name());
        vValueList.push_back(strValue);
    }

    if (vKeyList.size() != vValueList.size()) {
        return false;
    }

    std::string strKey = GetRecordCacheKey(self);
    if (!pDriver->HMSET(strKey, vKeyList, vValueList)) {
        return false;
    }

    if (nExpireSecond > 0) {
        pDriver->EXPIRE(strKey, nExpireSecond);
    }

    return true;
}

bool CommonRedisModule::GetPropertyList(const std::string &self, const std::vector<std::string> &fields, std::vector<std::string> &values) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return false;
    }

    std::string strCacheKey = GetPropertyCacheKey(self);
    if (pDriver->HMGET(strCacheKey, fields, values)) {
        return true;
    }

    return false;
}

INT64 CommonRedisModule::GetPropertyInt(const std::string &self, const std::string &propertyName) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return 0;
    }

    std::string strValue;
    std::string strCacheKey = GetPropertyCacheKey(self);
    if (!pDriver->HGET(strCacheKey, propertyName, strValue)) {
        return 0;
    }

    return lexical_cast<INT64>(strValue);
}

int CommonRedisModule::GetPropertyInt32(const std::string &self, const std::string &propertyName) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return 0;
    }

    std::string strValue;
    std::string strCacheKey = GetPropertyCacheKey(self);
    if (!pDriver->HGET(strCacheKey, propertyName, strValue)) {
        return 0;
    }

    return lexical_cast<INT64>(strValue);
}

double CommonRedisModule::GetPropertyFloat(const std::string &self, const std::string &propertyName) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return 0;
    }

    std::string strValue;
    std::string strCacheKey = GetPropertyCacheKey(self);
    if (!pDriver->HGET(strCacheKey, propertyName, strValue)) {
        return 0;
    }

    return lexical_cast<double>(strValue);
}

std::string CommonRedisModule::GetPropertyString(const std::string &self, const std::string &propertyName) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return "";
    }

    std::string strValue;
    std::string strCacheKey = GetPropertyCacheKey(self);
    if (!pDriver->HGET(strCacheKey, propertyName, strValue)) {
        return "";
    }

    return strValue;
}

Guid CommonRedisModule::GetPropertyObject(const std::string &self, const std::string &propertyName) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return Guid();
    }

    std::string strValue;
    std::string strCacheKey = GetPropertyCacheKey(self);
    if (!pDriver->HGET(strCacheKey, propertyName, strValue)) {
        return Guid();
    }
    Guid xID;
    xID.FromString(strValue);
    return xID;
}

Vector2 CommonRedisModule::GetPropertyVector2(const std::string &self, const std::string &propertyName) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return Vector2();
    }

    std::string strValue;
    std::string strCacheKey = GetPropertyCacheKey(self);
    if (!pDriver->HGET(strCacheKey, propertyName, strValue)) {
        return Vector2();
    }

    Vector2 xValue;
    xValue.FromString(strValue);
    return xValue;
}

Vector3 CommonRedisModule::GetPropertyVector3(const std::string &self, const std::string &propertyName) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return Vector3();
    }

    std::string strValue;
    std::string strCacheKey = GetPropertyCacheKey(self);
    if (!pDriver->HGET(strCacheKey, propertyName, strValue)) {
        return Vector3();
    }

    Vector3 xValue;
    xValue.FromString(strValue);
    return xValue;
}

bool CommonRedisModule::ConvertVectorToPropertyManager(std::vector<std::string> &vKeyList, std::vector<std::string> &vValueList,
                                                       std::shared_ptr<IPropertyManager> pPropertyManager, const bool cache, const bool save) {
    if (vKeyList.size() == vValueList.size()) {
        for (int i = 0; i < vKeyList.size(); ++i) {
            const std::string &strKey = vKeyList[i];
            const std::string &value = vValueList[i];
            if (value.empty()) {
                continue;
            }

            // std::cout << strKey << ":" << strValue << std::endl;

            std::shared_ptr<IProperty> pProperty = pPropertyManager->GetElement(strKey);
            if ((cache && pProperty->GetCache()) || (save && pProperty->GetSave())) {
                if (!pProperty->FromString(value)) {
                    // TODO
                    // error
                }
            }
        }
    }

    return true;
}

bool CommonRedisModule::ConvertVectorToRecordManager(std::vector<std::string> &vKeyList, std::vector<std::string> &vValueList,
                                                     std::shared_ptr<IRecordManager> pRecordManager, const bool cache, const bool save) {
    if (vKeyList.size() == vValueList.size()) {
        for (int i = 0; i < vKeyList.size(); ++i) {
            const std::string &strKey = vKeyList[i];
            const std::string &value = vValueList[i];
            if (value.empty()) {
                continue;
            }

            std::shared_ptr<IRecord> pRecord = pRecordManager->GetElement(strKey);
            if ((cache && pRecord->GetCache()) || (save && pRecord->GetSave())) {
                rpc::ObjectRecordBase xRecordData;
                if (xRecordData.ParseFromString(value)) {
                    ConvertPBToRecord(xRecordData, pRecord);
                } else {
                    // TODO
                    // error
                }
            }
        }
    }

    return true;
}

bool CommonRedisModule::ConvertPropertyManagerToVector(std::shared_ptr<IPropertyManager> pPropertyManager, std::vector<std::string> &vKeyList,
                                                       std::vector<std::string> &vValueList, const bool cache, const bool save) {
    for (std::shared_ptr<IProperty> pProperty = pPropertyManager->First(); pProperty != NULL; pProperty = pPropertyManager->Next()) {
        // const int type = pProperty->GetType();
        if ((cache && pProperty->GetCache()) || (save && pProperty->GetSave())) {
            const std::string &propertyName = pProperty->GetKey();
            const std::string &strPropertyValue = pProperty->ToString();

            // std::cout << propertyName << ":" << strPropertyValue << std::endl;

            vKeyList.push_back(propertyName);
            vValueList.push_back(strPropertyValue);
        }
    }

    return true;
}

bool CommonRedisModule::ConvertRecordManagerToVector(std::shared_ptr<IRecordManager> pRecordManager, std::vector<std::string> &vKeyList,
                                                     std::vector<std::string> &vValueList, const bool cache, const bool save) {
    for (std::shared_ptr<IRecord> pRecord = pRecordManager->First(); pRecord != NULL; pRecord = pRecordManager->Next()) {
        if ((cache && pRecord->GetCache()) || (save && pRecord->GetSave())) {
            rpc::ObjectRecordBase xRecordData;

            ConvertRecordToPB(pRecord, &xRecordData);

            ////
            std::string strValue;
            if (!xRecordData.SerializeToString(&strValue)) {
                continue;
            }

            vKeyList.push_back(xRecordData.record_name());
            vValueList.push_back(strValue);
        }
    }

    return true;
}

// bool CommonRedisModule::ConvertRecordToPB(const std::shared_ptr<IRecord> pRecord, rpc::ObjectRecordBase * pRecordData)

// bool CommonRedisModule::ConvertPBToRecord(const rpc::ObjectRecordBase& pRecordData, std::shared_ptr<IRecord> pRecord)

// bool CommonRedisModule::ConvertRecordManagerToPB(const std::shared_ptr<IRecordManager> pRecordManager, rpc::ObjectRecordList * pRecordDataList,
// const bool cache, const bool save)

// bool CommonRedisModule::ConvertPBToRecordManager(const rpc::ObjectRecordList & pRecordData, std::shared_ptr<IRecordManager> pRecord)

// bool CommonRedisModule::ConvertPropertyManagerToPB(const std::shared_ptr<IPropertyManager> pProps, rpc::ObjectPropertyList * pPropertyData, const
// bool cache, const bool save)

// bool CommonRedisModule::ConvertPBToPropertyManager(const rpc::ObjectPropertyList& pPropertyData, std::shared_ptr<IPropertyManager> pProps)

bool CommonRedisModule::SavePropertyInfo(const std::string &self, const std::string &propertyName, const std::string &propertyValue) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return false;
    }

    std::vector<std::string> vKeyList;
    std::vector<std::string> vValueList;

    vKeyList.push_back(propertyName);
    vValueList.push_back(propertyValue);

    if (vKeyList.size() != vValueList.size()) {
        return false;
    }

    std::string strKey = GetPropertyCacheKey(self);

    if (!pDriver->HMSET(strKey, vKeyList, vValueList)) {
        return false;
    }

    return false;
}

bool CommonRedisModule::GetPropertyList(const std::string &self, std::vector<std::pair<std::string, std::string>> &values) {
    std::shared_ptr<IRedisClient> pDriver = m_redis_->GetDriverBySuit(self);
    if (!pDriver) {
        return false;
    }

    std::string strCacheKey = GetPropertyCacheKey(self);
    if (pDriver->HGETALL(strCacheKey, values)) {
        return true;
    }

    return false;
}
