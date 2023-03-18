#pragma once
#include <squick/core/i_module.h>
#include <squick/core/i_property_manager.h>
#include <squick/core/i_record_manager.h>
class ICommonRedisModule : public IModule {
  public:
    virtual std::string GetPropertyCacheKey(const std::string &self) = 0;
    virtual std::string GetRecordCacheKey(const std::string &self) = 0;
    virtual std::string GetFriendCacheKey(const std::string &self) = 0;
    virtual std::string GetFriendInviteCacheKey(const std::string &self) = 0;
    virtual std::string GetBlockCacheKey(const std::string &self) = 0;

    virtual std::string GetTeamCacheKey(const std::string &self) = 0;
    virtual std::string GetTeamInviteCacheKey(const std::string &self) = 0;

    virtual std::string GetAccountCacheKey(const std::string &account) = 0;
    virtual std::string GetTileCacheKey(const int &sceneID) = 0;
    virtual std::string GetSceneCacheKey(const int &sceneID) = 0;
    virtual std::string GetCellCacheKey(const std::string &strCellID) = 0;

    virtual SQUICK_SHARE_PTR<IPropertyManager> NewPropertyManager(const std::string &className) = 0;
    virtual SQUICK_SHARE_PTR<IRecordManager> NewRecordManager(const std::string &className) = 0;

    virtual SQUICK_SHARE_PTR<IPropertyManager> GetPropertyInfo(const std::string &self, const std::string &className, const bool cache, const bool save,
                                                               SQUICK_SHARE_PTR<IPropertyManager> propertyManager = nullptr) = 0;
    virtual SQUICK_SHARE_PTR<IRecordManager> GetRecordInfo(const std::string &self, const std::string &className, const bool cache, const bool save,
                                                           SQUICK_SHARE_PTR<IRecordManager> recordManager = nullptr) = 0;

    virtual bool SavePropertyInfo(const std::string &self, const std::string &propertyName, const std::string &propertyValue) = 0;
    virtual bool SavePropertyInfo(const std::string &self, SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager, const bool cache, const bool save,
                                  const int nExpireSecond = -1) = 0;
    virtual bool SaveRecordInfo(const std::string &self, SQUICK_SHARE_PTR<IRecordManager> pRecordManager, const bool cache, const bool save,
                                const int nExpireSecond = -1) = 0;

    virtual bool GetPropertyList(const std::string &self, const std::vector<std::string> &fields, std::vector<std::string> &values) = 0;
    virtual bool GetPropertyList(const std::string &self, std::vector<std::pair<std::string, std::string>> &values) = 0;

    virtual INT64 GetPropertyInt(const std::string &self, const std::string &propertyName) = 0;
    virtual int GetPropertyInt32(const std::string &self, const std::string &propertyName) = 0;
    virtual double GetPropertyFloat(const std::string &self, const std::string &propertyName) = 0;
    virtual std::string GetPropertyString(const std::string &self, const std::string &propertyName) = 0;
    virtual Guid GetPropertyObject(const std::string &self, const std::string &propertyName) = 0;
    virtual Vector2 GetPropertyVector2(const std::string &self, const std::string &propertyName) = 0;
    virtual Vector3 GetPropertyVector3(const std::string &self, const std::string &propertyName) = 0;
};
