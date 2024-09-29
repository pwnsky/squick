#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <list>
#include <core/data_list.h>
#include <core/guid.h>
#include <core/i_module.h>
#include <core/i_property.h>
#include <core/i_property_manager.h>
#include <core/i_record.h>
#include <core/i_record_manager.h>
#include <core/list.h>
#include <core/map.h>
#include <core/vector4.h>

typedef std::function<int(const DataList &, const DataList &)> OBJECT_ENTER_EVENT_FUNCTOR;
typedef std::shared_ptr<OBJECT_ENTER_EVENT_FUNCTOR> OBJECT_ENTER_EVENT_FUNCTOR_PTR; // ObjectEnterCallBack

typedef std::function<int(const DataList &, const DataList &)> OBJECT_LEAVE_EVENT_FUNCTOR;
typedef std::shared_ptr<OBJECT_LEAVE_EVENT_FUNCTOR> OBJECT_LEAVE_EVENT_FUNCTOR_PTR; // ObjectLeaveCallBack

typedef std::function<int(const DataList &, const Guid &)> PROPERTY_ENTER_EVENT_FUNCTOR;
typedef std::shared_ptr<PROPERTY_ENTER_EVENT_FUNCTOR> PROPERTY_ENTER_EVENT_FUNCTOR_PTR; // AddPropertyEnterCallBack

typedef std::function<int(const DataList &, const Guid &)> RECORD_ENTER_EVENT_FUNCTOR;
typedef std::shared_ptr<RECORD_ENTER_EVENT_FUNCTOR> RECORD_ENTER_EVENT_FUNCTOR_PTR; // AddRecordEnterCallBack

typedef std::function<int(const Guid &, const std::string &, const SquickData &, const SquickData &, const DataList &, const INT64 reason)>
    PROPERTY_SINGLE_EVENT_FUNCTOR;
typedef std::shared_ptr<PROPERTY_SINGLE_EVENT_FUNCTOR> PROPERTY_SINGLE_EVENT_FUNCTOR_PTR; // AddPropertyEventCallBack

typedef std::function<int(const Guid &, const std::string &, const RECORD_EVENT_DATA &, const SquickData &, const SquickData &, const DataList &)>
    RECORD_SINGLE_EVENT_FUNCTOR;
typedef std::shared_ptr<RECORD_SINGLE_EVENT_FUNCTOR> RECORD_SINGLE_EVENT_FUNCTOR_PTR; // AddRecordEventCallBack

typedef std::function<int(const Guid &, const int, const int, const int, const DataList &)> SCENE_EVENT_FUNCTOR;
typedef std::shared_ptr<SCENE_EVENT_FUNCTOR> SCENE_EVENT_FUNCTOR_PTR;

class SceneInfo;
class SceneGroupInfo;

class ISceneModule : public IModule, public MapEx<int, SceneInfo> {
  public:
    virtual ~ISceneModule() { ClearAll(); }

    /////////////these interfaces below are for scene & group//////////////////
    template <typename BaseType>
    bool AddGroupPropertyCallBack(const std::string &propertyName, BaseType *pBase,
                                  int (BaseType::*handler)(const Guid &, const std::string &, const SquickData &, const SquickData &)) {
        PROPERTY_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        PROPERTY_EVENT_FUNCTOR_PTR functorPtr(new PROPERTY_EVENT_FUNCTOR(functor));
        return AddGroupPropertyCallBack(propertyName, functorPtr);
    }

    template <typename BaseType>
    bool AddGroupRecordCallBack(const std::string &recordName, BaseType *pBase,
                                int (BaseType::*handler)(const Guid &, const RECORD_EVENT_DATA &, const SquickData &, const SquickData &)) {
        RECORD_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        RECORD_EVENT_FUNCTOR_PTR functorPtr(new RECORD_EVENT_FUNCTOR(functor));
        return AddGroupRecordCallBack(recordName, functorPtr);
    }

    template <typename BaseType>
    bool AddGroupPropertyCommCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const std::string &, const SquickData &, const SquickData &)) {
        PROPERTY_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        PROPERTY_EVENT_FUNCTOR_PTR functorPtr(new PROPERTY_EVENT_FUNCTOR(functor));
        return AddGroupPropertyCommCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddGroupRecordCommCallBack(BaseType *pBase,
                                    int (BaseType::*handler)(const Guid &, const RECORD_EVENT_DATA &, const SquickData &, const SquickData &)) {
        RECORD_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        RECORD_EVENT_FUNCTOR_PTR functorPtr(new RECORD_EVENT_FUNCTOR(functor));
        return AddGroupRecordCommCallBack(functorPtr);
    }

    /////////////these interfaces below are for player//////////////////

    template <typename BaseType> bool AddObjectEnterCallBack(BaseType *pBase, int (BaseType::*handler)(const DataList &, const DataList &)) {
        OBJECT_ENTER_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2);
        OBJECT_ENTER_EVENT_FUNCTOR_PTR functorPtr(new OBJECT_ENTER_EVENT_FUNCTOR(functor));
        return AddObjectEnterCallBack(functorPtr);
    }
    template <typename BaseType> bool AddObjectDataFinishedCallBack(BaseType *pBase, int (BaseType::*handler)(const DataList &, const DataList &)) {
        OBJECT_ENTER_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2);
        OBJECT_ENTER_EVENT_FUNCTOR_PTR functorPtr(new OBJECT_ENTER_EVENT_FUNCTOR(functor));
        return AddObjectDataFinishedCallBack(functorPtr);
    }
    template <typename BaseType> bool AddObjectLeaveCallBack(BaseType *pBase, int (BaseType::*handler)(const DataList &, const DataList &)) {
        OBJECT_LEAVE_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2);
        OBJECT_LEAVE_EVENT_FUNCTOR_PTR functorPtr(new OBJECT_LEAVE_EVENT_FUNCTOR(functor));
        return AddObjectLeaveCallBack(functorPtr);
    }

    template <typename BaseType> bool AddPropertyEnterCallBack(BaseType *pBase, int (BaseType::*handler)(const DataList &, const Guid &)) {
        PROPERTY_ENTER_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2);
        PROPERTY_ENTER_EVENT_FUNCTOR_PTR functorPtr(new PROPERTY_ENTER_EVENT_FUNCTOR(functor));
        return AddPropertyEnterCallBack(functorPtr);
    }

    template <typename BaseType> bool AddRecordEnterCallBack(BaseType *pBase, int (BaseType::*handler)(const DataList &, const Guid &)) {
        RECORD_ENTER_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2);
        RECORD_ENTER_EVENT_FUNCTOR_PTR functorPtr(new RECORD_ENTER_EVENT_FUNCTOR(functor));
        return AddRecordEnterCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddPropertyEventCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const std::string &, const SquickData &, const SquickData &,
                                                                            const DataList &, const INT64 reason)) {
        PROPERTY_SINGLE_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                                          std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);
        PROPERTY_SINGLE_EVENT_FUNCTOR_PTR functorPtr(new PROPERTY_SINGLE_EVENT_FUNCTOR(functor));
        return AddPropertyEventCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddRecordEventCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const std::string &, const RECORD_EVENT_DATA &, const SquickData &,
                                                                          const SquickData &, const DataList &)) {
        RECORD_SINGLE_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                                        std::placeholders::_4, std::placeholders::_5, std::placeholders::_6);
        RECORD_SINGLE_EVENT_FUNCTOR_PTR functorPtr(new RECORD_SINGLE_EVENT_FUNCTOR(functor));
        return AddRecordEventCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddEnterSceneConditionCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const int, const int, const DataList &)) {
        SCENE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        SCENE_EVENT_FUNCTOR_PTR functorPtr(new SCENE_EVENT_FUNCTOR(functor));
        return AddEnterSceneConditionCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddBeforeEnterSceneGroupCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const int, const int, const DataList &)) {
        SCENE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        SCENE_EVENT_FUNCTOR_PTR functorPtr(new SCENE_EVENT_FUNCTOR(functor));
        return AddBeforeEnterSceneGroupCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddAfterEnterSceneGroupCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const int, const int, const DataList &)) {
        SCENE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        SCENE_EVENT_FUNCTOR_PTR functorPtr(new SCENE_EVENT_FUNCTOR(functor));
        return AddAfterEnterSceneGroupCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddAfterEnterAndReadySceneGroupCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const int, const int, const DataList &)) {
        SCENE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        SCENE_EVENT_FUNCTOR_PTR functorPtr(new SCENE_EVENT_FUNCTOR(functor));
        return AddAfterEnterAndReadySceneGroupCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddSwapSceneEventCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const int, const int, const DataList &)) {
        SCENE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        SCENE_EVENT_FUNCTOR_PTR functorPtr(new SCENE_EVENT_FUNCTOR(functor));
        return AddSwapSceneEventCallBack(functorPtr);
    }

    virtual bool RemoveSwapSceneEventCallBack() = 0;

    template <typename BaseType>
    bool AddBeforeLeaveSceneGroupCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const int, const int, const DataList &)) {
        SCENE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        SCENE_EVENT_FUNCTOR_PTR functorPtr(new SCENE_EVENT_FUNCTOR(functor));
        return AddBeforeLeaveSceneGroupCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddAfterLeaveSceneGroupCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const int, const int, const DataList &)) {
        SCENE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        SCENE_EVENT_FUNCTOR_PTR functorPtr(new SCENE_EVENT_FUNCTOR(functor));
        return AddAfterLeaveSceneGroupCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddSceneGroupCreatedCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const int, const int, const DataList &)) {
        SCENE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        SCENE_EVENT_FUNCTOR_PTR functorPtr(new SCENE_EVENT_FUNCTOR(functor));
        return AddSceneGroupCreatedCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddSceneGroupDestroyedCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const int, const int, const DataList &)) {
        SCENE_EVENT_FUNCTOR functor =
            std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
        SCENE_EVENT_FUNCTOR_PTR functorPtr(new SCENE_EVENT_FUNCTOR(functor));
        return AddSceneGroupDestroyedCallBack(functorPtr);
    }

    virtual int RequestGroupScene(const int sceneID) = 0;
    virtual bool RequestEnterScene(const Guid &self, const int sceneID, const int groupID, const int type, const Vector3 &pos, const DataList &argList) = 0;
    virtual bool ReleaseGroupScene(const int sceneID, const int groupID) = 0;
    virtual bool LeaveSceneGroup(const Guid &self) = 0;
    virtual const std::vector<int> &GetGroups(const int sceneID) = 0;

    virtual bool AddSeedData(const int sceneID, const std::string &seedID, const std::string &configID, const Vector3 &vPos, const int nHeight) = 0;
    virtual const Vector3 &GetSeedPos(const int sceneID, const std::string &seedID) = 0;
    virtual const int GetSeedPWeight(const int sceneID, const std::string &seedID) = 0;

    virtual bool AddRelivePosition(const int sceneID, const int nIndex, const Vector3 &vPos) = 0;
    virtual const Vector3 &GetRelivePosition(const int sceneID, const int nIndex = -1) = 0;
    virtual bool AddTagPosition(const int sceneID, const int nIndex, const Vector3 &vPos) = 0;
    virtual const Vector3 &GetTagPosition(const int sceneID, const int nIndex = -1) = 0;

    virtual bool CreateSceneNPC(const int sceneID, const int groupID) = 0;
    virtual bool CreateSceneNPC(const int sceneID, const int groupID, const DataList &argList) = 0;
    virtual bool DestroySceneNPC(const int sceneID, const int groupID) = 0;

    /////////////the interfaces below are for scene & group/////////////////////////////
    virtual bool SetPropertyInt(const int scene, const int group, const std::string &propertyName, const INT64 nValue) = 0;
    virtual bool SetPropertyFloat(const int scene, const int group, const std::string &propertyName, const double dValue) = 0;
    virtual bool SetPropertyString(const int scene, const int group, const std::string &propertyName, const std::string &value) = 0;
    virtual bool SetPropertyObject(const int scene, const int group, const std::string &propertyName, const Guid &objectValue) = 0;
    virtual bool SetPropertyVector2(const int scene, const int group, const std::string &propertyName, const Vector2 &value) = 0;
    virtual bool SetPropertyVector3(const int scene, const int group, const std::string &propertyName, const Vector3 &value) = 0;

    virtual INT64 GetPropertyInt(const int scene, const int group, const std::string &propertyName) = 0;
    virtual int GetPropertyInt32(const int scene, const int group,
                                 const std::string &propertyName) = 0; // equal to (int)GetPropertyInt(...), to remove C4244 warning
    virtual double GetPropertyFloat(const int scene, const int group, const std::string &propertyName) = 0;
    virtual const std::string &GetPropertyString(const int scene, const int group, const std::string &propertyName) = 0;
    virtual const Guid &GetPropertyObject(const int scene, const int group, const std::string &propertyName) = 0;
    virtual const Vector2 &GetPropertyVector2(const int scene, const int group, const std::string &propertyName) = 0;
    virtual const Vector3 &GetPropertyVector3(const int scene, const int group, const std::string &propertyName) = 0;

    virtual std::shared_ptr<IPropertyManager> FindPropertyManager(const int scene, const int group) = 0;
    virtual std::shared_ptr<IRecordManager> FindRecordManager(const int scene, const int group) = 0;
    virtual std::shared_ptr<IRecord> FindRecord(const int scene, const int group, const std::string &recordName) = 0;
    virtual bool ClearRecord(const int scene, const int group, const std::string &recordName) = 0;

    virtual bool SetRecordInt(const int scene, const int group, const std::string &recordName, const int row, const int col, const INT64 nValue) = 0;
    virtual bool SetRecordFloat(const int scene, const int group, const std::string &recordName, const int row, const int col, const double dwValue) = 0;
    virtual bool SetRecordString(const int scene, const int group, const std::string &recordName, const int row, const int col, const std::string &value) = 0;
    virtual bool SetRecordObject(const int scene, const int group, const std::string &recordName, const int row, const int col, const Guid &objectValue) = 0;
    virtual bool SetRecordVector2(const int scene, const int group, const std::string &recordName, const int row, const int col, const Vector2 &value) = 0;
    virtual bool SetRecordVector3(const int scene, const int group, const std::string &recordName, const int row, const int col, const Vector3 &value) = 0;

    virtual bool SetRecordInt(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag, const INT64 value) = 0;
    virtual bool SetRecordFloat(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag,
                                const double value) = 0;
    virtual bool SetRecordString(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag,
                                 const std::string &value) = 0;
    virtual bool SetRecordObject(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag,
                                 const Guid &value) = 0;
    virtual bool SetRecordVector2(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag,
                                  const Vector2 &value) = 0;
    virtual bool SetRecordVector3(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag,
                                  const Vector3 &value) = 0;

    virtual INT64 GetRecordInt(const int scene, const int group, const std::string &recordName, const int row, const int col) = 0;
    virtual double GetRecordFloat(const int scene, const int group, const std::string &recordName, const int row, const int col) = 0;
    virtual const std::string &GetRecordString(const int scene, const int group, const std::string &recordName, const int row, const int col) = 0;
    virtual const Guid &GetRecordObject(const int scene, const int group, const std::string &recordName, const int row, const int col) = 0;
    virtual const Vector2 &GetRecordVector2(const int scene, const int group, const std::string &recordName, const int row, const int col) = 0;
    virtual const Vector3 &GetRecordVector3(const int scene, const int group, const std::string &recordName, const int row, const int col) = 0;

    virtual INT64 GetRecordInt(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual double GetRecordFloat(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const std::string &GetRecordString(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const Guid &GetRecordObject(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const Vector2 &GetRecordVector2(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const Vector3 &GetRecordVector3(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) = 0;

    ////////////////////////////////////////////////////////////////
  protected:
    // for scene && group
    virtual bool AddGroupRecordCallBack(const std::string &name, const RECORD_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddGroupPropertyCallBack(const std::string &name, const PROPERTY_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddGroupRecordCommCallBack(const RECORD_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddGroupPropertyCommCallBack(const PROPERTY_EVENT_FUNCTOR_PTR &cb) = 0;

    // for players
    virtual bool AddObjectEnterCallBack(const OBJECT_ENTER_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddObjectLeaveCallBack(const OBJECT_LEAVE_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddPropertyEnterCallBack(const PROPERTY_ENTER_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddRecordEnterCallBack(const RECORD_ENTER_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddPropertyEventCallBack(const PROPERTY_SINGLE_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddRecordEventCallBack(const RECORD_SINGLE_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddObjectDataFinishedCallBack(const OBJECT_ENTER_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual bool AddEnterSceneConditionCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual bool AddBeforeEnterSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddAfterEnterSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddAfterEnterAndReadySceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddSwapSceneEventCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddBeforeLeaveSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddAfterLeaveSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual bool AddSceneGroupCreatedCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) = 0;
    virtual bool AddSceneGroupDestroyedCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) = 0;

  private:
};