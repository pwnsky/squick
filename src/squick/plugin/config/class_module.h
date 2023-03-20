
#ifndef SQUICK_LOGICCLASS_MODULE_H
#define SQUICK_LOGICCLASS_MODULE_H

#include "element_module.h"
#include "i_class_module.h"
#include "i_element_module.h"
#include <iostream>
#include <map>
#include <squick/core/data_list.h>
#include <squick/core/i_plugin_manager.h>
#include <squick/core/list.h>
#include <squick/core/map.h>
#include <squick/core/property_manager.h>
#include <squick/core/record.h>
#include <squick/core/record_manager.h>
#include <string>
#include <third_party/rapidxml/rapidxml.hpp>
#include <thread>

class Class : public IClass {
  public:
    Class(const std::string &className) {
        mParentClass = NULL;
        mClassName = className;

        mPropertyManager = SQUICK_SHARE_PTR<IPropertyManager>(SQUICK_NEW PropertyManager(Guid()));
        mRecordManager = SQUICK_SHARE_PTR<IRecordManager>(SQUICK_NEW RecordManager(Guid()));
    }

    virtual ~Class() { ClearAll(); }

    virtual SQUICK_SHARE_PTR<IPropertyManager> GetPropertyManager() { return mPropertyManager; }

    virtual SQUICK_SHARE_PTR<IRecordManager> GetRecordManager() { return mRecordManager; }

    virtual bool AddClassCallBack(const CLASS_EVENT_FUNCTOR_PTR &cb) { return mClassEventInfo.Add(cb); }

    virtual bool DoEvent(const Guid &objectID, const CLASS_OBJECT_EVENT classEvent, const DataList &valueList) {
        CLASS_EVENT_FUNCTOR_PTR cb;
        bool bRet = mClassEventInfo.First(cb);
        while (bRet) {
            cb->operator()(objectID, mClassName, classEvent, valueList);

            bRet = mClassEventInfo.Next(cb);
        }

        return true;
    }

    void SetParent(SQUICK_SHARE_PTR<IClass> pClass) { mParentClass = pClass; }

    SQUICK_SHARE_PTR<IClass> GetParent() { return mParentClass; }

    void SetTypeName(const char *strType) { mType = strType; }

    const std::string &GetTypeName() { return mType; }

    const std::string &GetClassName() { return mClassName; }

    const bool AddId(std::string &strId) {
        mIdList.push_back(strId);
        return true;
    }

    const std::vector<std::string> &GetIDList() { return mIdList; }

    void SetInstancePath(const std::string &strPath) { mClassInstancePath = strPath; }

    const std::string &GetInstancePath() { return mClassInstancePath; }

  private:
    SQUICK_SHARE_PTR<IPropertyManager> mPropertyManager;
    SQUICK_SHARE_PTR<IRecordManager> mRecordManager;

    SQUICK_SHARE_PTR<IClass> mParentClass;
    std::string mType;
    std::string mClassName;
    std::string mClassInstancePath;

    std::vector<std::string> mIdList;

    List<CLASS_EVENT_FUNCTOR_PTR> mClassEventInfo;
};

class ClassModule : public IClassModule {
  private:
    ClassModule();

  public:
    ClassModule(IPluginManager *p);
    virtual ~ClassModule() override;

    virtual bool Awake() override;
    virtual bool Start() override;
    virtual bool AfterStart() override;

    virtual bool Destory() override;

    virtual bool Load() override;
    virtual bool Save() override;
    virtual bool Clear() override;

    virtual IClassModule *GetThreadClassModule() override;
    virtual IClassModule *GetThreadClassModule(const int index) override;

    virtual bool AddClassCallBack(const std::string &className, const CLASS_EVENT_FUNCTOR_PTR &cb) override;
    virtual bool DoEvent(const Guid &objectID, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &valueList) override;

    virtual SQUICK_SHARE_PTR<IPropertyManager> GetClassPropertyManager(const std::string &className) override;
    virtual SQUICK_SHARE_PTR<IRecordManager> GetClassRecordManager(const std::string &className) override;

    virtual bool AddClass(const std::string &className, const std::string &strParentName) override;

  protected:
    virtual DATA_TYPE ComputerType(const char *pstrTypeName, SquickData &var);
    virtual bool AddProperties(rapidxml::xml_node<> *pPropertyRootNode, SQUICK_SHARE_PTR<IClass> pClass);
    virtual bool AddRecords(rapidxml::xml_node<> *pRecordRootNode, SQUICK_SHARE_PTR<IClass> pClass);
    virtual bool AddComponents(rapidxml::xml_node<> *pRecordRootNode, SQUICK_SHARE_PTR<IClass> pClass);
    virtual bool AddClassInclude(const char *pstrClassFilePath, SQUICK_SHARE_PTR<IClass> pClass);
    virtual bool AddClass(const char *pstrClassFilePath, SQUICK_SHARE_PTR<IClass> pClass);

    virtual bool Load(rapidxml::xml_node<> *attrNode, SQUICK_SHARE_PTR<IClass> pParentClass);

  protected:
    struct ThreadClassModule {
        bool used;
        std::thread::id threadID;
        ClassModule *classModule;
    };

    std::vector<ThreadClassModule> mThreadClasses;

  protected:
    std::string mConfigFileName;
    bool mbBackup = false;
};

#endif
