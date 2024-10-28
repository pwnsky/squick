#pragma once

#include "i_common_config_module.h"
#include <core/i_plugin_manager.h>
#include <core/map_ex.h>
#include <core/platform.h>
class ConfigModule : public ICommonConfigModule {
  public:
  public:
    class CAttributeList : public MapEx<std::string, std::string> {
      public:
        const int GetInt(const std::string &name) {
            std::shared_ptr<std::string> pStr = GetElement(name);
            if (pStr && !pStr->empty()) {
                int nData = 0;
                SQUICK_StrTo(*pStr, nData);
                return nData;
            }
            return 0;
        }

        const std::string &GetString(const std::string &name) {
            std::shared_ptr<std::string> pStr = GetElement(name);
            if (pStr) {
                return *pStr;
            }
            return std::string();
        }
    };

    struct CStructInfo : public MapEx<std::string, CAttributeList> {
        CAttributeList mmStructAttribute;
    };

  public:
    ConfigModule(IPluginManager *p) { pm_ = p; }
    virtual ~ConfigModule(){};

    virtual bool Start();
    virtual bool Destroy();
    virtual bool Update();
    virtual bool AfterStart();

    virtual bool ClearConfig();
    virtual bool LoadConfig(const std::string &strFile);

    virtual const int GetFieldInt(const std::string &strStructName, const std::string &strStructItemName, const std::string &strAttribute);
    virtual const int GetFieldInt(const std::string &strStructName, const std::string &strSDKAttribute);

    virtual const std::string &GetFieldString(const std::string &strStructName, const std::string &strStructItemName, const std::string &strAttribute);
    virtual const std::string &GetFieldString(const std::string &strStructName, const std::string &strSDKAttribute);

    virtual std::vector<std::string> GetSubKeyList(const std::string &strStructName);
    virtual std::vector<std::string> GetFieldList(const std::string &strStructName);

  private:
    MapEx<std::string, CStructInfo> mmData; // strStructName<-->CStructInfo
};