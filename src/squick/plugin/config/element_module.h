
#ifndef SQUICK_ELEMENT_MODULE_H
#define SQUICK_ELEMENT_MODULE_H

#include "i_class_module.h"
#include "i_element_module.h"
#include <iostream>
#include <map>
#include <squick/core/data_list.h>
#include <squick/core/list.h>
#include <squick/core/map.h>
#include <squick/core/property_manager.h>
#include <squick/core/record.h>
#include <squick/core/record_manager.h>
#include <squick/plugin/log/i_log_module.h>
#include <string>
#include <third_party/rapidxml/rapidxml.hpp>
#include <third_party/rapidxml/rapidxml_iterators.hpp>
#include <third_party/rapidxml/rapidxml_print.hpp>
#include <third_party/rapidxml/rapidxml_utils.hpp>
#include <thread>

class Class;

class ElementConfigInfo {
  public:
    ElementConfigInfo() {
        m_pPropertyManager = SQUICK_SHARE_PTR<IPropertyManager>(SQUICK_NEW PropertyManager(Guid()));
        m_pRecordManager = SQUICK_SHARE_PTR<IRecordManager>(SQUICK_NEW RecordManager(Guid()));
    }

    virtual ~ElementConfigInfo() {}

    SQUICK_SHARE_PTR<IPropertyManager> GetPropertyManager() { return m_pPropertyManager; }

    SQUICK_SHARE_PTR<IRecordManager> GetRecordManager() { return m_pRecordManager; }

  protected:
    // std::string mstrConfigID;

    SQUICK_SHARE_PTR<IPropertyManager> m_pPropertyManager;
    SQUICK_SHARE_PTR<IRecordManager> m_pRecordManager;
};

class ElementModule : public IElementModule, MapEx<std::string, ElementConfigInfo> {
  private:
    ElementModule(ElementModule *p);

  public:
    ElementModule(IPluginManager *p);
    virtual ~ElementModule();

    virtual bool Awake() override;
    virtual bool Start() override;
    virtual bool Destory() override;

    virtual bool AfterStart() override;
    virtual bool BeforeDestory() override;
    virtual bool Update() override;

    virtual bool Load() override;
    virtual bool Save() override;
    virtual bool Clear() override;

    IElementModule *GetThreadElementModule() override;

    virtual bool LoadSceneInfo(const std::string &fileName, const std::string &className) override;

    virtual bool ExistElement(const std::string &configName) override;
    virtual bool ExistElement(const std::string &className, const std::string &configName) override;

    virtual SQUICK_SHARE_PTR<IPropertyManager> GetPropertyManager(const std::string &configName) override;
    virtual SQUICK_SHARE_PTR<IRecordManager> GetRecordManager(const std::string &configName) override;

    virtual INT64 GetPropertyInt(const std::string &configName, const std::string &propertyName) override;
    virtual int GetPropertyInt32(const std::string &configName, const std::string &propertyName) override;
    virtual double GetPropertyFloat(const std::string &configName, const std::string &propertyName) override;
    virtual const std::string &GetPropertyString(const std::string &configName, const std::string &propertyName) override;
    virtual const Vector2 GetPropertyVector2(const std::string &configName, const std::string &propertyName) override;
    virtual const Vector3 GetPropertyVector3(const std::string &configName, const std::string &propertyName) override;

    virtual const std::vector<std::string> GetListByProperty(const std::string &className, const std::string &propertyName, const INT64 nValue) override;
    virtual const std::vector<std::string> GetListByProperty(const std::string &className, const std::string &propertyName, const std::string &nValue) override;

  protected:
    virtual SQUICK_SHARE_PTR<IProperty> GetProperty(const std::string &configName, const std::string &propertyName);

    virtual bool Load(rapidxml::xml_node<> *attrNode, SQUICK_SHARE_PTR<IClass> pLogicClass);
    virtual bool CheckRef();
    virtual bool LegalNumber(const char *str);
    virtual bool LegalFloat(const char *str);

  protected:
    struct ThreadElementModule {
        bool used;
        std::thread::id threadID;
        ElementModule *elementModule;
    };

    std::vector<ThreadElementModule> mThreadElements;
    ElementModule *originalElementModule;

  protected:
    IClassModule *m_pClassModule;
    ILogModule *m_pLogModule;

    bool mbLoaded;
    bool mbBackup = false;
};

#endif
