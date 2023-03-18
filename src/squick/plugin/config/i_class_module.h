
#ifndef SQUICK_INTF_LOGICCLASS_MODULE_H
#define SQUICK_INTF_LOGICCLASS_MODULE_H

#include <iostream>
#include <squick/core/i_module.h>
#include <squick/core/i_object.h>

typedef std::function<int(const Guid &, const std::string &, const CLASS_OBJECT_EVENT, const DataList &)> CLASS_EVENT_FUNCTOR;
typedef SQUICK_SHARE_PTR<CLASS_EVENT_FUNCTOR> CLASS_EVENT_FUNCTOR_PTR; // EVENT

class IClass : public List<std::string> // include files
{
  public:
    virtual ~IClass() {}

    virtual SQUICK_SHARE_PTR<IPropertyManager> GetPropertyManager() = 0;

    virtual SQUICK_SHARE_PTR<IRecordManager> GetRecordManager() = 0;

    virtual void SetParent(SQUICK_SHARE_PTR<IClass> pClass) = 0;
    virtual SQUICK_SHARE_PTR<IClass> GetParent() = 0;
    virtual void SetTypeName(const char *strType) = 0;
    virtual const std::string &GetTypeName() = 0;
    virtual const std::string &GetClassName() = 0;
    virtual const bool AddId(std::string &configName) = 0;
    virtual const std::vector<std::string> &GetIDList() = 0;
    virtual const std::string &GetInstancePath() = 0;
    virtual void SetInstancePath(const std::string &strPath) = 0;

    virtual bool AddClassCallBack(const CLASS_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool DoEvent(const Guid &objectID, const CLASS_OBJECT_EVENT classEvent, const DataList &valueList) = 0;
};

class IClassModule : public IModule, public MapEx<std::string, IClass> {
  public:
    virtual ~IClassModule() {}
    virtual bool Load() = 0;
    virtual bool Save() = 0;
    virtual bool Clear() = 0;

    virtual IClassModule *GetThreadClassModule() = 0;
    virtual IClassModule *GetThreadClassModule(const int index) = 0;

    template <typename BaseType>
    bool AddClassCallBack(const std::string &className, BaseType *pBase,
                          int (BaseType::*handler)(const Guid &, const std::string &, const CLASS_OBJECT_EVENT, const DataList &)) {
        CLASS_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        CLASS_EVENT_FUNCTOR_PTR functorPtr(new CLASS_EVENT_FUNCTOR(functor));
        return AddClassCallBack(className, functorPtr);
    }

    virtual bool DoEvent(const Guid &objectID, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &valueList) = 0;

    virtual bool AddClassCallBack(const std::string &className, const CLASS_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual SQUICK_SHARE_PTR<IPropertyManager> GetClassPropertyManager(const std::string &className) = 0;

    virtual SQUICK_SHARE_PTR<IRecordManager> GetClassRecordManager(const std::string &className) = 0;

    virtual bool AddClass(const std::string &className, const std::string &strParentName) = 0;
};

#endif