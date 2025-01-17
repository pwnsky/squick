#pragma once

#include <iostream>
#include <core/data_list.h>
#include <core/i_module.h>
#include <core/property_manager.h>
#include <core/record_manager.h>

class IElementModule : public IModule {
  public:
    virtual bool Load() = 0;
    virtual bool Save() = 0;
    virtual bool Clear() = 0;

    virtual IElementModule *GetThreadElementModule() = 0;

    virtual const std::string GetConfigId(const std::string &class_name, const std::string &id) = 0;

    // special
    virtual bool LoadSceneInfo(const std::string &fileName, const std::string &className) = 0;

    virtual bool ExistElement(const std::string &configName) = 0;
    virtual bool ExistElement(const std::string &className, const std::string &configName) = 0;

    virtual std::shared_ptr<IPropertyManager> GetPropertyManager(const std::string &configName) = 0;
    virtual std::shared_ptr<IRecordManager> GetRecordManager(const std::string &configName) = 0;

    virtual INT64 GetPropertyInt(const std::string &configName, const std::string &propertyName) = 0;
    virtual int GetPropertyInt32(const std::string &configName, const std::string &propertyName) = 0;
    virtual double GetPropertyFloat(const std::string &configName, const std::string &propertyName) = 0;
    virtual const std::string &GetPropertyString(const std::string &configName, const std::string &propertyName) = 0;
    virtual const Vector2 GetPropertyVector2(const std::string &configName, const std::string &propertyName) = 0;
    virtual const Vector3 GetPropertyVector3(const std::string &configName, const std::string &propertyName) = 0;

    virtual const std::vector<std::string> GetListByProperty(const std::string &className, const std::string &propertyName, const INT64 nValue) = 0;
    virtual const std::vector<std::string> GetListByProperty(const std::string &className, const std::string &propertyName, const std::string &nValue) = 0;
};