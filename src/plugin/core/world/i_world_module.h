#pragma once

#include <functional>
#include <iostream>
#include <core/base.h>
#include <core/guid.h>
#include <core/i_module.h>
#include <core/i_object.h>
#include <plugin/core/config/export.h>
#include <string>
class IWorldModule : public IModule {

  public:
    template <typename BaseType>
    bool AddPropertyCallBack(const Guid &self, const std::string &propertyName, BaseType *pBase,
                             int (BaseType::*handler)(const Guid &, const std::string &, const SquickData &, const SquickData &, const INT64)) {
        std::shared_ptr<IObject> pObject = GetObject(self);
        if (pObject.get()) {
            return pObject->AddPropertyCallBack(propertyName, pBase, handler);
        }

        return false;
    }

    template <typename BaseType>
    bool AddRecordCallBack(const Guid &self, const std::string &recordName, BaseType *pBase,
                           int (BaseType::*handler)(const Guid &, const RECORD_EVENT_DATA &, const SquickData &, const SquickData &)) {
        std::shared_ptr<IObject> pObject = GetObject(self);
        if (pObject.get()) {
            return pObject->AddRecordCallBack(recordName, pBase, handler);
        }

        return false;
    }

    ////////////////event//////////////////////////////////////////////////////////
    template <typename BaseType>
    bool AddClassCallBack(const std::string &className, BaseType *pBase,
                          int (BaseType::*handler)(const Guid &, const std::string &, const CLASS_OBJECT_EVENT, const DataList &)) {
        CLASS_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        CLASS_EVENT_FUNCTOR_PTR functorPtr(new CLASS_EVENT_FUNCTOR(functor));
        return AddClassCallBack(className, functorPtr);
    }

    virtual bool DoEvent(const Guid &self, const std::string &className, CLASS_OBJECT_EVENT eEvent, const DataList &valueList) = 0;

    //////////////////////////////////////////////////////////////////////////
    template <typename BaseType>
    bool RegisterCommonClassEvent(BaseType *pBase, int (BaseType::*handler)(const Guid &, const std::string &, const CLASS_OBJECT_EVENT, const DataList &)) {
        CLASS_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        CLASS_EVENT_FUNCTOR_PTR functorPtr(new CLASS_EVENT_FUNCTOR(functor));
        return RegisterCommonClassEvent(functorPtr);
    }

    template <typename BaseType>
    bool RegisterCommonPropertyEvent(BaseType *pBase,
                                     int (BaseType::*handler)(const Guid &, const std::string &, const SquickData &, const SquickData &, const INT64)) {
        PROPERTY_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        PROPERTY_EVENT_FUNCTOR_PTR functorPtr(new PROPERTY_EVENT_FUNCTOR(functor));
        return RegisterCommonPropertyEvent(functorPtr);
    }

    template <typename BaseType>
    bool RegisterCommonRecordEvent(BaseType *pBase, int (BaseType::*handler)(const Guid &, const RECORD_EVENT_DATA &, const SquickData &, const SquickData &)) {
        RECORD_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        RECORD_EVENT_FUNCTOR_PTR functorPtr(new RECORD_EVENT_FUNCTOR(functor));
        return RegisterCommonRecordEvent(functorPtr);
    }

    template <typename BaseType>
    bool RegisterClassPropertyEvent(const std::string &className, BaseType *pBase,
                                    int (BaseType::*handler)(const Guid &, const std::string &, const SquickData &, const SquickData &)) {
        PROPERTY_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        PROPERTY_EVENT_FUNCTOR_PTR functorPtr(new PROPERTY_EVENT_FUNCTOR(functor));
        return RegisterClassPropertyEvent(className, functorPtr);
    }

    template <typename BaseType>
    bool RegisterClassRecordEvent(const std::string &className, BaseType *pBase,
                                  int (BaseType::*handler)(const Guid &, const RECORD_EVENT_DATA &, const SquickData &, const SquickData &)) {
        RECORD_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        RECORD_EVENT_FUNCTOR_PTR functorPtr(new RECORD_EVENT_FUNCTOR(functor));
        return RegisterClassRecordEvent(className, functorPtr);
    }

    /////////////////////////////////////////////////////////////////

    virtual bool ExistScene(const int sceneID) = 0;

    virtual bool ExistObject(const Guid &ident) = 0;
    virtual bool ObjectReady(const Guid &ident) = 0;
    virtual bool ExistObject(const Guid &ident, const int sceneID, const int groupID) = 0;

    virtual std::shared_ptr<IObject> GetObject(const Guid &ident) = 0;
    virtual std::shared_ptr<IObject> CreateObject(const Guid &self, const int sceneID, const int groupID, const std::string &className,
                                                  const std::string &configIndex, const DataList &arg) = 0;

    virtual bool DestroySelf(const Guid &self) = 0;
    virtual bool DestroyObject(const Guid &self) = 0;
    virtual bool DestroyAll() = 0;

    virtual bool FindProperty(const Guid &self, const std::string &propertyName) = 0;

    virtual bool SetPropertyInt(const Guid &self, const std::string &propertyName, const INT64 nValue, const INT64 reason = 0) = 0;
    virtual bool SetPropertyFloat(const Guid &self, const std::string &propertyName, const double dValue, const INT64 reason = 0) = 0;
    virtual bool SetPropertyString(const Guid &self, const std::string &propertyName, const std::string &value, const INT64 reason = 0) = 0;
    virtual bool SetPropertyObject(const Guid &self, const std::string &propertyName, const Guid &objectValue, const INT64 reason = 0) = 0;
    virtual bool SetPropertyVector2(const Guid &self, const std::string &propertyName, const Vector2 &value, const INT64 reason = 0) = 0;
    virtual bool SetPropertyVector3(const Guid &self, const std::string &propertyName, const Vector3 &value, const INT64 reason = 0) = 0;

    virtual INT64 GetPropertyInt(const Guid &self, const std::string &propertyName) = 0;
    virtual int GetPropertyInt32(const Guid &self, const std::string &propertyName) = 0;
    virtual double GetPropertyFloat(const Guid &self, const std::string &propertyName) = 0;
    virtual const std::string &GetPropertyString(const Guid &self, const std::string &propertyName) = 0;
    virtual const Guid &GetPropertyObject(const Guid &self, const std::string &propertyName) = 0;
    virtual const Vector2 &GetPropertyVector2(const Guid &self, const std::string &propertyName) = 0;
    virtual const Vector3 &GetPropertyVector3(const Guid &self, const std::string &propertyName) = 0;

    virtual std::shared_ptr<IRecord> FindRecord(const Guid &self, const std::string &recordName) = 0;
    virtual bool ClearRecord(const Guid &self, const std::string &recordName) = 0;

    virtual bool SetRecordInt(const Guid &self, const std::string &recordName, const int row, const int col, const INT64 nValue) = 0;
    virtual bool SetRecordFloat(const Guid &self, const std::string &recordName, const int row, const int col, const double dwValue) = 0;
    virtual bool SetRecordString(const Guid &self, const std::string &recordName, const int row, const int col, const std::string &value) = 0;
    virtual bool SetRecordObject(const Guid &self, const std::string &recordName, const int row, const int col, const Guid &objectValue) = 0;
    virtual bool SetRecordVector2(const Guid &self, const std::string &recordName, const int row, const int col, const Vector2 &value) = 0;
    virtual bool SetRecordVector3(const Guid &self, const std::string &recordName, const int row, const int col, const Vector3 &value) = 0;

    virtual bool SetRecordInt(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const INT64 value) = 0;
    virtual bool SetRecordFloat(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const double value) = 0;
    virtual bool SetRecordString(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const std::string &value) = 0;
    virtual bool SetRecordObject(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Guid &value) = 0;
    virtual bool SetRecordVector2(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Vector2 &value) = 0;
    virtual bool SetRecordVector3(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Vector3 &value) = 0;

    virtual INT64 GetRecordInt(const Guid &self, const std::string &recordName, const int row, const int col) = 0;
    inline int GetRecordInt32(const Guid &self, const std::string &recordName, const int row, const int col) {
        return (int)GetRecordInt(self, recordName, row, col);
    };
    virtual double GetRecordFloat(const Guid &self, const std::string &recordName, const int row, const int col) = 0;
    virtual const std::string &GetRecordString(const Guid &self, const std::string &recordName, const int row, const int col) = 0;
    virtual const Guid &GetRecordObject(const Guid &self, const std::string &recordName, const int row, const int col) = 0;
    virtual const Vector2 &GetRecordVector2(const Guid &self, const std::string &recordName, const int row, const int col) = 0;
    virtual const Vector3 &GetRecordVector3(const Guid &self, const std::string &recordName, const int row, const int col) = 0;

    virtual INT64 GetRecordInt(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) = 0;
    inline int GetRecordInt32(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
        return (int)GetRecordInt(self, recordName, row, colTag);
    };
    virtual double GetRecordFloat(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const std::string &GetRecordString(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const Guid &GetRecordObject(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const Vector2 &GetRecordVector2(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const Vector3 &GetRecordVector3(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) = 0;

    ////////////////////////////////////////////////////////////////
    virtual Guid CreateGUID() = 0;
    virtual Guid CreatePlayerGUID() = 0;

    virtual bool CreateScene(const int sceneID) = 0;
    virtual bool DestroyScene(const int sceneID) = 0;

    virtual int GetOnLineCount() = 0;
    virtual int GetMaxOnLineCount() = 0;

    virtual int RequestGroupScene(const int sceneID) = 0;
    virtual bool ReleaseGroupScene(const int sceneID, const int groupID) = 0;
    virtual bool ExitGroupScene(const int sceneID, const int groupID) = 0;

    virtual int GetGroupObjectList(const int sceneID, const int groupID, const bool bPlayer, const Guid &noSelf) = 0;
    virtual bool GetGroupObjectList(const int sceneID, const int groupID, DataList &list) = 0;
    virtual bool GetGroupObjectList(const int sceneID, const int groupID, DataList &list, const Guid &noSelf) = 0;
    virtual bool GetGroupObjectList(const int sceneID, const int groupID, DataList &list, const bool bPlayer) = 0;
    virtual bool GetGroupObjectList(const int sceneID, const int groupID, DataList &list, const bool bPlayer, const Guid &noSelf) = 0;
    virtual bool GetGroupObjectList(const int sceneID, const int groupID, const std::string &className, DataList &list) = 0;
    virtual bool GetGroupObjectList(const int sceneID, const int groupID, const std::string &className, DataList &list, const Guid &noSelf) = 0;

    virtual int GetObjectByProperty(const int sceneID, const int groupID, const std::string &propertyName, const DataList &valueArg, DataList &list) = 0;

    // 0-10 ==>0,1,2,3,4,5,6,7,8,9
    virtual int Random(int nStart, int nEnd) = 0;
    virtual float Random() = 0;
    virtual bool LogInfo(const Guid ident) = 0;

  protected:
    virtual bool AddClassCallBack(const std::string &className, const CLASS_EVENT_FUNCTOR_PTR &cb) = 0;

  protected:
    virtual bool RegisterCommonClassEvent(const CLASS_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool RegisterCommonPropertyEvent(const PROPERTY_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool RegisterCommonRecordEvent(const RECORD_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual bool RegisterClassPropertyEvent(const std::string &className, const PROPERTY_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool RegisterClassRecordEvent(const std::string &className, const RECORD_EVENT_FUNCTOR_PTR &cb) = 0;
};