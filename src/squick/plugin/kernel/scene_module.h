

#ifndef SQUICK_SCENE_MODULE_H
#define SQUICK_SCENE_MODULE_H

#include <iostream>
#include <squick/core/i_object.h>
#include <squick/core/guid.h>
#include <squick/plugin/log/i_log_module.h>

#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include "i_kernel_module.h"
#include "i_event_module.h"
#include "i_cell_module.h"
#include "i_scene_module.h"
#include <forward_list>

// all object in this group
/*
if a object in the group of '0', them it can be see by all object in this scene.
*/
struct SceneSeedResource
{
	std::string seedID;
	std::string configID;
	Vector3 vSeedPos;
	int nWeight;
};

struct TransformSync
{
	Guid 	owner;
	Vector3 position;
	Vector3 rotation;
	int32_t object_id;
};

class SceneGroupInfo
{
public:
	SceneGroupInfo(const int sceneID, const int groupID, SQUICK_SHARE_PTR<IPropertyManager> xPropertyManager, SQUICK_SHARE_PTR<IRecordManager> xRecordManager)
	{
		this->groupID = groupID;
		this->mxPropertyManager = xPropertyManager;
		this->mxRecordManager = xRecordManager;
	}

	virtual ~SceneGroupInfo()
	{
	}

	bool Update()
	{
		return true;
	}

	MapEx<Guid, int> mxPlayerList;
	MapEx<Guid, int> mxOtherList;
	int groupID;

	int sequence = 0;
	std::map<Guid, std::forward_list<TransformSync>> mPlayerPosition;
	SQUICK_SHARE_PTR<IPropertyManager> mxPropertyManager;
	SQUICK_SHARE_PTR<IRecordManager> mxRecordManager;
};

// all group in this scene
class SceneInfo
	: public MapEx<int, SceneGroupInfo>
{
public:

	SceneInfo(const int sceneID)
	{
		this->groupIndex = -1;
		this->sceneID = sceneID;
		this->width = 512;
	}

	SceneInfo(const int sceneID, const int nWidth)
	{
		this->groupIndex = -1;
		this->sceneID = sceneID;
		this->width = nWidth;
	}

	virtual ~SceneInfo()
	{
		ClearAll();
	}

	int NewGroupID()
	{
		groupIndex += 1;
		return groupIndex;
	}

	int GetWidth()
	{
		return width;
	}

	bool AddObjectToGroup(const int groupID, const Guid& ident, bool bPlayer)
	{
		SQUICK_SHARE_PTR<SceneGroupInfo> pInfo = GetElement(groupID);
		if (pInfo.get())
		{
			if (bPlayer)
			{
				return pInfo->mxPlayerList.AddElement(ident, SQUICK_SHARE_PTR<int>(new int(0)));
			}
			else
			{
				return pInfo->mxOtherList.AddElement(ident, SQUICK_SHARE_PTR<int>(new int(0)));
			}
		}

		return false;
	}

	bool RemoveObjectFromGroup(const int groupID, const Guid& ident, bool bPlayer)
	{
		SQUICK_SHARE_PTR<SceneGroupInfo> pInfo = GetElement(groupID);
		if (pInfo)
		{
			if (bPlayer)
			{
				return pInfo->mxPlayerList.RemoveElement(ident);
			}
			else
			{
				return pInfo->mxOtherList.RemoveElement(ident);
			}
		}

		return false;
	}

	bool ExistObjectInGroup(const int groupID, const Guid& ident)
	{
		SQUICK_SHARE_PTR<SceneGroupInfo> pInfo = GetElement(groupID);
		if (pInfo)
		{
			return pInfo->mxPlayerList.ExistElement(ident) || pInfo->mxOtherList.ExistElement(ident);
		}

		return false;
	}

	bool Update()
	{
		SQUICK_SHARE_PTR<SceneGroupInfo> pGroupInfo = First();
		while (pGroupInfo.get())
		{
			pGroupInfo->Update();

			pGroupInfo = Next();
		}
		return true;
	}

	bool AddSeedObjectInfo(const std::string& seedID, const std::string& configID, const Vector3& vPos, const int nWeight)
	{
		SQUICK_SHARE_PTR<SceneSeedResource> pInfo = mtSceneResourceConfig.GetElement(seedID);
		if (!pInfo)
		{
			pInfo = SQUICK_SHARE_PTR<SceneSeedResource>(new SceneSeedResource());
			pInfo->seedID = seedID;
			pInfo->configID = configID;
			pInfo->vSeedPos = vPos;
			pInfo->nWeight = nWeight;
			return mtSceneResourceConfig.AddElement(seedID, pInfo);
		}

		return true;
	}

	SQUICK_SHARE_PTR<SceneSeedResource> GetSeedObjectInfo(const std::string& seedID)
	{
		return mtSceneResourceConfig.GetElement(seedID);
	}

	bool RemoveSeedObject(const std::string& seedID)
	{
		return true;
	}

	bool AddReliveInfo(const int nIndex, const Vector3& vPos)
	{
		return mtSceneRelivePos.AddElement(nIndex, SQUICK_SHARE_PTR<Vector3>(SQUICK_NEW Vector3(vPos)));
	}

	const Vector3& GetReliveInfo(const int nIndex)
	{
		if (mtSceneRelivePos.Count() > 0)
		{
			if (nIndex < 0)
			{
				int rd = rand() % mtSceneRelivePos.Count();
				return *(mtSceneRelivePos.GetElement(rd));
			}

			if (nIndex < mtSceneRelivePos.Count())
			{
				SQUICK_SHARE_PTR<Vector3> vPos = mtSceneRelivePos.GetElement(nIndex);
				if (vPos)
				{
					return *vPos;
				}
			}
		}

		return Vector3::Zero();
	}

	bool AddTagInfo(const int nIndex, const Vector3& vPos)
	{
		return mtTagPos.AddElement(nIndex, SQUICK_SHARE_PTR<Vector3>(SQUICK_NEW Vector3(vPos)));
	}

	const Vector3& GetTagInfo(const int nIndex)
	{
		if (mtTagPos.Count() > 0)
		{
			if (nIndex < 0)
			{
				int rd = rand() % mtTagPos.Count();
				return *(mtTagPos.GetElement(rd));
			}

			if (nIndex < mtTagPos.Count())
			{
				SQUICK_SHARE_PTR<Vector3> vPos = mtTagPos.GetElement(nIndex);
				if (vPos)
				{
					return *vPos;
				}
			}
		}

		return Vector3::Zero();
	}

	int groupIndex;
	int sceneID;
	int width;
	//seedID, seedInfo
	MapEx<std::string, SceneSeedResource > mtSceneResourceConfig;
	MapEx<int, Vector3 > mtSceneRelivePos;
	MapEx<int, Vector3 > mtTagPos;
};


class SceneModule
    : public ISceneModule
{
public:
	SceneModule(IPluginManager* p)
    {
        pPluginManager = p;
    }


    virtual ~SceneModule()
    {
    }

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool BeforeDestory();
    virtual bool Destory();
    virtual bool Update();

	virtual int RequestGroupScene(const int sceneID);
	virtual bool RequestEnterScene(const Guid& self, const int sceneID, const int groupID, const int type, const Vector3& pos, const DataList& argList);
	virtual bool ReleaseGroupScene(const int sceneID, const int groupID);
	virtual bool LeaveSceneGroup(const Guid& self);
	virtual const std::vector<int>& GetGroups(const int sceneID);

	virtual bool AddSeedData(const int sceneID, const std::string& seedID, const std::string& configID, const Vector3& vPos, const int nWeight);
	virtual const Vector3& GetSeedPos(const int sceneID, const std::string& seedID);
	virtual const int GetSeedPWeight(const int sceneID, const std::string& seedID);

	virtual bool AddRelivePosition(const int sceneID, const int nIndex, const Vector3& vPos);
	virtual const Vector3& GetRelivePosition(const int sceneID, const int nIndex = -1);

	virtual bool AddTagPosition(const int sceneID, const int nIndex, const Vector3& vPos);
	virtual const Vector3& GetTagPosition(const int sceneID, const int nIndex= -1);

	virtual bool CreateSceneNPC(const int sceneID, const int groupID);
	virtual bool CreateSceneNPC(const int sceneID, const int groupID, const DataList& argList);
	virtual bool DestroySceneNPC(const int sceneID, const int groupID);

	virtual bool RemoveSwapSceneEventCallBack();

	//////////////////////////////////////////////////////////////////////////
    virtual bool SetPropertyInt(const int scene, const int group, const std::string& propertyName, const INT64 nValue);
    virtual bool SetPropertyFloat(const int scene, const int group, const std::string& propertyName, const double dValue);
    virtual bool SetPropertyString(const int scene, const int group, const std::string& propertyName, const std::string& value);
    virtual bool SetPropertyObject(const int scene, const int group, const std::string& propertyName, const Guid& objectValue);
	virtual bool SetPropertyVector2(const int scene, const int group, const std::string& propertyName, const Vector2& value);
	virtual bool SetPropertyVector3(const int scene, const int group, const std::string& propertyName, const Vector3& value);

    virtual INT64 GetPropertyInt(const int scene, const int group, const std::string& propertyName);
	virtual int GetPropertyInt32(const int scene, const int group, const std::string& propertyName);	//equal to (int)GetPropertyInt(...), to remove C4244 warning
    virtual double GetPropertyFloat(const int scene, const int group, const std::string& propertyName);
    virtual const std::string& GetPropertyString(const int scene, const int group, const std::string& propertyName);
    virtual const Guid& GetPropertyObject(const int scene, const int group, const std::string& propertyName);
	virtual const Vector2& GetPropertyVector2(const int scene, const int group, const std::string& propertyName);
	virtual const Vector3& GetPropertyVector3(const int scene, const int group, const std::string& propertyName);

    //////////////////////////////////////////////////////////////////////////
	virtual SQUICK_SHARE_PTR<IPropertyManager> FindPropertyManager(const int scene, const int group);
	virtual SQUICK_SHARE_PTR<IRecordManager> FindRecordManager(const int scene, const int group);
    virtual SQUICK_SHARE_PTR<IRecord> FindRecord(const int scene, const int group, const std::string& recordName);
    virtual bool ClearRecord(const int scene, const int group, const std::string& recordName);

    virtual bool SetRecordInt(const int scene, const int group, const std::string& recordName, const int row, const int col, const INT64 nValue);
    virtual bool SetRecordFloat(const int scene, const int group, const std::string& recordName, const int row, const int col, const double dwValue);
    virtual bool SetRecordString(const int scene, const int group, const std::string& recordName, const int row, const int col, const std::string& value);
    virtual bool SetRecordObject(const int scene, const int group, const std::string& recordName, const int row, const int col, const Guid& objectValue);
	virtual bool SetRecordVector2(const int scene, const int group, const std::string& recordName, const int row, const int col, const Vector2& value);
	virtual bool SetRecordVector3(const int scene, const int group, const std::string& recordName, const int row, const int col, const Vector3& value);

    virtual bool SetRecordInt(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag, const INT64 value);
    virtual bool SetRecordFloat(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag, const double value);
    virtual bool SetRecordString(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag, const std::string& value);
    virtual bool SetRecordObject(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag, const Guid& value);
	virtual bool SetRecordVector2(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag, const Vector2& value);
	virtual bool SetRecordVector3(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag, const Vector3& value);

    virtual INT64 GetRecordInt(const int scene, const int group, const std::string& recordName, const int row, const int col);
    virtual double GetRecordFloat(const int scene, const int group, const std::string& recordName, const int row, const int col);
    virtual const std::string& GetRecordString(const int scene, const int group, const std::string& recordName, const int row, const int col);
    virtual const Guid& GetRecordObject(const int scene, const int group, const std::string& recordName, const int row, const int col);
	virtual const Vector2& GetRecordVector2(const int scene, const int group, const std::string& recordName, const int row, const int col);
	virtual const Vector3& GetRecordVector3(const int scene, const int group, const std::string& recordName, const int row, const int col);

    virtual INT64 GetRecordInt(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag);
    virtual double GetRecordFloat(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag);
    virtual const std::string& GetRecordString(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag);
    virtual const Guid& GetRecordObject(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag);
	virtual const Vector2& GetRecordVector2(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag);
	virtual const Vector3& GetRecordVector3(const int scene, const int group, const std::string& recordName, const int row, const std::string& colTag);

    ////////////////////////////////////////////////////////////////
protected:
	//for scene && group
	virtual bool AddGroupPropertyCallBack(const std::string& name, const PROPERTY_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddGroupRecordCallBack(const std::string& name, const RECORD_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddGroupPropertyCommCallBack(const PROPERTY_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddGroupRecordCommCallBack(const RECORD_EVENT_FUNCTOR_PTR& cb);

	//for players
	virtual bool AddObjectEnterCallBack(const OBJECT_ENTER_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddObjectDataFinishedCallBack(const OBJECT_ENTER_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddObjectLeaveCallBack(const OBJECT_LEAVE_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddPropertyEnterCallBack(const PROPERTY_ENTER_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddRecordEnterCallBack(const RECORD_ENTER_EVENT_FUNCTOR_PTR& cb);

	virtual bool AddPropertyEventCallBack(const PROPERTY_SINGLE_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddRecordEventCallBack(const RECORD_SINGLE_EVENT_FUNCTOR_PTR& cb);

	virtual bool AddEnterSceneConditionCallBack(const SCENE_EVENT_FUNCTOR_PTR& cb);

	virtual bool AddBeforeEnterSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddAfterEnterSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddAfterEnterAndReadySceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddSwapSceneEventCallBack(const SCENE_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddBeforeLeaveSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddAfterLeaveSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR& cb);

	virtual bool AddSceneGroupCreatedCallBack(const SCENE_EVENT_FUNCTOR_PTR& cb);
	virtual bool AddSceneGroupDestroyedCallBack(const SCENE_EVENT_FUNCTOR_PTR& cb);
protected:
	bool SwitchScene(const Guid& self, const int nTargetSceneID, const int nTargetGroupID, const int type, const Vector3 v, const float fOrient, const DataList& arg);

protected:
	//for scene && group
	int OnScenePropertyCommonEvent(const Guid& self, const std::string& propertyName, const SquickData& oldVar, const SquickData& newVar, const INT64 reason);
	int OnSceneRecordCommonEvent(const Guid& self, const RECORD_EVENT_DATA& eventData, const SquickData& oldVar, const SquickData& newVar);


	//for players
	int OnPropertyCommonEvent(const Guid& self, const std::string& propertyName, const SquickData& oldVar, const SquickData& newVar, const INT64 reason);
	int OnRecordCommonEvent(const Guid& self, const RECORD_EVENT_DATA& eventData, const SquickData& oldVar, const SquickData& newVar);
	int OnClassCommonEvent(const Guid& self, const std::string& className, const CLASS_OBJECT_EVENT classEvent, const DataList& var);

	int OnPlayerGroupEvent(const Guid& self, const std::string& propertyName, const SquickData& oldVar, const SquickData& newVar);
	int OnPlayerSceneEvent(const Guid& self, const std::string& propertyName, const SquickData& oldVar, const SquickData& newVar);
	
	int GetBroadCastObject(const Guid& self, const std::string& propertyName, const bool bTable, DataList& valueObject);

	int EnterSceneCondition(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);

	int BeforeLeaveSceneGroup(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);
	int AfterLeaveSceneGroup(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);
	int OnSwapSceneEvent(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);
	int BeforeEnterSceneGroup(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);
	int AfterEnterSceneGroup(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);
	int AfterEnterAndReadySceneGroup(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);

	int SceneGroupCreatedEvent(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);
	int SceneGroupDestroyedEvent(const Guid& self, const int sceneID, const int groupID, const int type, const DataList& argList);


protected:
	////////////////interface for broadcast event//////////////////////////////////


	//broad the data of argvar to self
	int OnObjectListEnter(const DataList& self, const DataList& argVar);
	int OnObjectListEnterFinished(const DataList& self, const DataList& argVar);

	int OnObjectListLeave(const DataList& self, const DataList& argVar);

	//broad the data of self to argvar 
	int OnPropertyEnter(const DataList& argVar, const Guid& self);
	int OnRecordEnter(const DataList& argVar, const Guid& self);

	int OnPropertyEvent(const Guid& self, const std::string& propertyName, const SquickData& oldVar, const SquickData& newVar, const DataList& argVar, const INT64 reason);
	int OnRecordEvent(const Guid& self, const std::string& recordName, const RECORD_EVENT_DATA& eventData, const SquickData& oldVar, const SquickData& newVar, const DataList& argVar);
	
	////////////////interface for broadcast event///////////////////////////////////
	int OnMoveCellEvent(const Guid& self, const int& scene, const int& group, const Guid& fromCell, const Guid& toCell);

private:

	//for scene & group
	std::list<PROPERTY_EVENT_FUNCTOR_PTR> mtGroupPropertyCommCallBackList;
	std::list<RECORD_EVENT_FUNCTOR_PTR> mtGroupRecordCallCommBackList;
	std::map<std::string, std::list<PROPERTY_EVENT_FUNCTOR_PTR>> mtGroupPropertyCallBackList;
	std::map<std::string, std::list<RECORD_EVENT_FUNCTOR_PTR>> mtGroupRecordCallBackList;

	//for players & objects
	std::vector<OBJECT_ENTER_EVENT_FUNCTOR_PTR> mvObjectEnterCallback;
	std::vector<OBJECT_ENTER_EVENT_FUNCTOR_PTR> mvObjectDataFinishedCallBack;
	std::vector<OBJECT_LEAVE_EVENT_FUNCTOR_PTR> mvObjectLeaveCallback;

	std::vector<PROPERTY_ENTER_EVENT_FUNCTOR_PTR> mvPropertyEnterCallback;
	std::vector<RECORD_ENTER_EVENT_FUNCTOR_PTR> mvRecordEnterCallback;

	std::vector<PROPERTY_SINGLE_EVENT_FUNCTOR_PTR> mvPropertySingleCallback;
	std::vector<RECORD_SINGLE_EVENT_FUNCTOR_PTR> mvRecordSingleCallback;

	std::vector<SCENE_EVENT_FUNCTOR_PTR> mvEnterSceneConditionCallback;
	std::vector<SCENE_EVENT_FUNCTOR_PTR> mvBeforeEnterSceneCallback;
	std::vector<SCENE_EVENT_FUNCTOR_PTR> mvAfterEnterSceneCallback;
	std::vector<SCENE_EVENT_FUNCTOR_PTR> mvAfterEnterAndReadySceneCallback;
	std::vector<SCENE_EVENT_FUNCTOR_PTR> mvOnSwapSceneCallback;
	std::vector<SCENE_EVENT_FUNCTOR_PTR> mvBeforeLeaveSceneCallback;
	std::vector<SCENE_EVENT_FUNCTOR_PTR> mvAfterLeaveSceneCallback;

	std::vector<SCENE_EVENT_FUNCTOR_PTR> mvSceneGroupCreatedCallback;
	std::vector<SCENE_EVENT_FUNCTOR_PTR> mvSceneGroupDestroyedCallback;

private:
	IKernelModule* m_pKernelModule;
	IClassModule* m_pClassModule;
	ILogModule* m_pLogModule;
	IElementModule* m_pElementModule;
	IEventModule* m_pEventModule;
	ICellModule* m_pCellModule;
};

#endif