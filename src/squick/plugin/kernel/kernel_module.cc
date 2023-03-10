
#include "kernel_module.h"
#include "scene_module.h"
#include <squick/core/mem_manager.h>
#include <squick/core/object.h>
#include <squick/core/record.h>
#include <squick/core/performance.h>
#include <squick/core/property_manager.h>
#include <squick/core/record_manager.h>
#include <squick/core/memory_counter.h>
#include <squick/core/guid.h>
#include <squick/struct/excel.h>

KernelModule::KernelModule(IPluginManager* p)
{
    m_bIsUpdate = true;
	nGUIDIndex = 0;
	nLastTime = 0;

	pPluginManager = p;

	nLastTime = pPluginManager->GetNowTime();
	StartRandom();
}

KernelModule::~KernelModule()
{
	ClearAll();
}

void KernelModule::StartRandom()
{
	mvRandom.clear();

	constexpr int nRandomMax = 100000;
	mvRandom.reserve(nRandomMax);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 1.0f);

	for (int i = 0; i < nRandomMax; i++)
	{
		mvRandom.emplace_back((float)dis(gen));
	}

	mxRandomItor = mvRandom.cbegin();
}

bool KernelModule::Start()
{
	mtDeleteSelfList.clear();

	m_pSceneModule = pPluginManager->FindModule<ISceneModule>();
	m_pClassModule = pPluginManager->FindModule<IClassModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();
	m_pScheduleModule = pPluginManager->FindModule<IScheduleModule>();
	m_pEventModule = pPluginManager->FindModule<IEventModule>();
	m_pCellModule = pPluginManager->FindModule<ICellModule>();
	m_pThreadPoolModule = pPluginManager->FindModule<IThreadPoolModule>();
	return true;
}

bool KernelModule::Destory()
{
	return true;
}

bool KernelModule::Update()
{
	ProcessMemFree();

	mnCurExeObject.nHead64 = 0;
	mnCurExeObject.nData64 = 0;

	if (mtDeleteSelfList.size() > 0)
	{
		std::list<Guid>::iterator it = mtDeleteSelfList.begin();
		for (; it != mtDeleteSelfList.end(); it++)
		{
			DestroyObject(*it);
		}
		mtDeleteSelfList.clear();
	}

	return true;
}

SQUICK_SHARE_PTR<IObject> KernelModule::CreateObject(const Guid& self, const int sceneID, const int groupID, const std::string& className, const std::string& configIndex, const DataList& arg)
{
	SQUICK_SHARE_PTR<IObject> pObject;
	Guid ident = self;

	SQUICK_SHARE_PTR<SceneInfo> pContainerInfo = m_pSceneModule->GetElement(sceneID);
	if (!pContainerInfo)
	{
		m_pLogModule->LogError(Guid(0, sceneID), "There is no scene " + std::to_string(sceneID), __FUNCTION__, __LINE__);
		return pObject;
	}

	if (!pContainerInfo->GetElement(groupID))
	{
		m_pLogModule->LogError("There is no group " + std::to_string(groupID), __FUNCTION__, __LINE__);
		return pObject;
	}

	//  if (!m_pElementModule->ExistElement(configIndex))
	//  {
	//      m_pLogModule->LogError(Guid(0, sceneID), "There is no group", groupID, __FUNCTION__, __LINE__);
	//      return pObject;
	//  }


	if (ident.IsNull())
	{
		ident = CreateGUID();
	}

	if (GetElement(ident))
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, ident, "The object has Exists", __FUNCTION__, __LINE__);
		return pObject;
	}

	pObject = SQUICK_SHARE_PTR<IObject>(SQUICK_NEW Object(ident, pPluginManager));
	AddElement(ident, pObject);

	if (pPluginManager->UsingBackThread())
	{
		m_pThreadPoolModule->DoAsyncTask(Guid(), "",
			[=](ThreadTask& task) -> void
			{
				//backup thread for async task
				{
					SQUICK_SHARE_PTR<IPropertyManager> pStaticClassPropertyManager = m_pClassModule->GetThreadClassModule()->GetClassPropertyManager(className);
					SQUICK_SHARE_PTR<IRecordManager> pStaticClassRecordManager = m_pClassModule->GetThreadClassModule()->GetClassRecordManager(className);
					if (pStaticClassPropertyManager && pStaticClassRecordManager)
					{
						SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
						SQUICK_SHARE_PTR<IRecordManager> pRecordManager = pObject->GetRecordManager();

						SQUICK_SHARE_PTR<IProperty> pStaticConfigPropertyInfo = pStaticClassPropertyManager->First();
						while (pStaticConfigPropertyInfo)
						{
							SQUICK_SHARE_PTR<IProperty> xProperty = pPropertyManager->AddProperty(ident, pStaticConfigPropertyInfo->GetKey(), pStaticConfigPropertyInfo->GetType());

							xProperty->SetPublic(pStaticConfigPropertyInfo->GetPublic());
							xProperty->SetPrivate(pStaticConfigPropertyInfo->GetPrivate());
							xProperty->SetSave(pStaticConfigPropertyInfo->GetSave());
							xProperty->SetCache(pStaticConfigPropertyInfo->GetCache());
							xProperty->SetRef(pStaticConfigPropertyInfo->GetRef());
							xProperty->SetUpload(pStaticConfigPropertyInfo->GetUpload());

							//
							pObject->AddPropertyCallBack(pStaticConfigPropertyInfo->GetKey(), this, &KernelModule::OnPropertyCommonEvent);

							pStaticConfigPropertyInfo = pStaticClassPropertyManager->Next();
						}

						SQUICK_SHARE_PTR<IRecord> pConfigRecordInfo = pStaticClassRecordManager->First();
						while (pConfigRecordInfo)
						{
							SQUICK_SHARE_PTR<IRecord> xRecord = pRecordManager->AddRecord(ident,
								pConfigRecordInfo->GetName(),
								pConfigRecordInfo->GetStartData(),
								pConfigRecordInfo->GetTag(),
								pConfigRecordInfo->GetRows());

							xRecord->SetPublic(pConfigRecordInfo->GetPublic());
							xRecord->SetPrivate(pConfigRecordInfo->GetPrivate());
							xRecord->SetSave(pConfigRecordInfo->GetSave());
							xRecord->SetCache(pConfigRecordInfo->GetCache());
							xRecord->SetUpload(pConfigRecordInfo->GetUpload());

							//
							pObject->AddRecordCallBack(pConfigRecordInfo->GetName(), this, &KernelModule::OnRecordCommonEvent);

							pConfigRecordInfo = pStaticClassRecordManager->Next();
						}
					}
				}
			},
			[=](ThreadTask& task) -> void
			{
				//no data--main thread
				{
					Vector3 vRelivePos = m_pSceneModule->GetRelivePosition(sceneID, 0);

					pObject->SetPropertyString(excel::IObject::ConfigID(), configIndex);
					pObject->SetPropertyString(excel::IObject::ClassName(), className);
					pObject->SetPropertyInt(excel::IObject::SceneID(), sceneID);
					pObject->SetPropertyInt(excel::IObject::GroupID(), groupID);
					pObject->SetPropertyVector3(excel::IObject::Position(), vRelivePos);

					pContainerInfo->AddObjectToGroup(groupID, ident, className == excel::Player::ThisName() ? true : false);

					DoEvent(ident, className, pObject->GetState(), arg);
				}

				m_pThreadPoolModule->DoAsyncTask(Guid(), "",
					[=](ThreadTask& task) -> void
					{
						//backup thread
						{
							SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
							SQUICK_SHARE_PTR<IPropertyManager> pConfigPropertyManager = m_pElementModule->GetThreadElementModule()->GetPropertyManager(configIndex);
							SQUICK_SHARE_PTR<IRecordManager> pConfigRecordManager = m_pElementModule->GetThreadElementModule()->GetRecordManager(configIndex);

							if (pConfigPropertyManager && pConfigRecordManager)
							{
								SQUICK_SHARE_PTR<IProperty> pConfigPropertyInfo = pConfigPropertyManager->First();
								while (nullptr != pConfigPropertyInfo)
								{
									if (pConfigPropertyInfo->Changed())
									{
										pPropertyManager->SetProperty(pConfigPropertyInfo->GetKey(), pConfigPropertyInfo->GetValue());
									}

									pConfigPropertyInfo = pConfigPropertyManager->Next();
								}
							}
						}
					},
					[=](ThreadTask& task) -> void
					{

						//main thread
						{
							SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
							for (int i = 0; i < arg.GetCount() - 1; i += 2)
							{
								const std::string& propertyName = arg.String(i);
								if (excel::IObject::ConfigID() != propertyName
									&& excel::IObject::ClassName() != propertyName
									&& excel::IObject::SceneID() != propertyName
									&& excel::IObject::ID() != propertyName
									&& excel::IObject::GroupID() != propertyName)
								{
									SQUICK_SHARE_PTR<IProperty> pArgProperty = pPropertyManager->GetElement(propertyName);
									if (pArgProperty)
									{
										switch (pArgProperty->GetType())
										{
										case TDATA_INT:
											pObject->SetPropertyInt(propertyName, arg.Int(i + 1));
											break;
										case TDATA_FLOAT:
											pObject->SetPropertyFloat(propertyName, arg.Float(i + 1));
											break;
										case TDATA_STRING:
											pObject->SetPropertyString(propertyName, arg.String(i + 1));
											break;
										case TDATA_OBJECT:
											pObject->SetPropertyObject(propertyName, arg.Object(i + 1));
											break;
										case TDATA_VECTOR2:
											pObject->SetPropertyVector2(propertyName, arg.Vector2At(i + 1));
											break;
										case TDATA_VECTOR3:
											pObject->SetPropertyVector3(propertyName, arg.Vector3At(i + 1));
											break;
										default:
											break;
										}
									}
								}
							}

							std::ostringstream stream;
							stream << " create object: " << ident.ToString();
							stream << " config_name: " << configIndex;
							stream << " scene_id: " << sceneID;
							stream << " group_id: " << groupID;
							stream << " position: " << pObject->GetPropertyVector3(excel::IObject::Position()).ToString();

							m_pLogModule->LogInfo(stream);

							pObject->SetState(COE_CREATE_BEFORE_ATTACHDATA);
							DoEvent(ident, className, pObject->GetState(), arg);

						}

						m_pThreadPoolModule->DoAsyncTask(Guid(), "",
							[=](ThreadTask& task) -> void
							{
								//back up thread
								{
									pObject->SetState(COE_CREATE_LOADDATA);
									DoEvent(ident, className, pObject->GetState(), arg);
								}
							},
							[=](ThreadTask& task) -> void
							{
								//below are main thread
								{
									pObject->SetState(COE_CREATE_AFTER_ATTACHDATA);
									DoEvent(ident, className, pObject->GetState(), arg);

									pObject->SetState(COE_CREATE_BEFORE_EFFECT);
									DoEvent(ident, className, pObject->GetState(), arg);

									pObject->SetState(COE_CREATE_EFFECTDATA);
									DoEvent(ident, className, pObject->GetState(), arg);

									pObject->SetState(COE_CREATE_AFTER_EFFECT);
									DoEvent(ident, className, pObject->GetState(), arg);

									pObject->SetState(COE_CREATE_READY);
									DoEvent(ident, className, pObject->GetState(), arg);

									pObject->SetState(COE_CREATE_HASDATA);
									DoEvent(ident, className, pObject->GetState(), arg);

									pObject->SetState(COE_CREATE_FINISH);
									DoEvent(ident, className, pObject->GetState(), arg);
								}
							});

					});
			});

	}
	else
	{
		//backup thread for async task
		{
			SQUICK_SHARE_PTR<IPropertyManager> pStaticClassPropertyManager = m_pClassModule->GetClassPropertyManager(className);
			SQUICK_SHARE_PTR<IRecordManager> pStaticClassRecordManager = m_pClassModule->GetClassRecordManager(className);
			if (pStaticClassPropertyManager && pStaticClassRecordManager)
			{
				SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
				SQUICK_SHARE_PTR<IRecordManager> pRecordManager = pObject->GetRecordManager();

				SQUICK_SHARE_PTR<IProperty> pStaticConfigPropertyInfo = pStaticClassPropertyManager->First();
				while (pStaticConfigPropertyInfo)
				{
					SQUICK_SHARE_PTR<IProperty> xProperty = pPropertyManager->AddProperty(ident, pStaticConfigPropertyInfo->GetKey(), pStaticConfigPropertyInfo->GetType());

					xProperty->SetPublic(pStaticConfigPropertyInfo->GetPublic());
					xProperty->SetPrivate(pStaticConfigPropertyInfo->GetPrivate());
					xProperty->SetSave(pStaticConfigPropertyInfo->GetSave());
					xProperty->SetCache(pStaticConfigPropertyInfo->GetCache());
					xProperty->SetRef(pStaticConfigPropertyInfo->GetRef());
					xProperty->SetUpload(pStaticConfigPropertyInfo->GetUpload());


					pObject->AddPropertyCallBack(pStaticConfigPropertyInfo->GetKey(), this, &KernelModule::OnPropertyCommonEvent);

					pStaticConfigPropertyInfo = pStaticClassPropertyManager->Next();
				}

				SQUICK_SHARE_PTR<IRecord> pConfigRecordInfo = pStaticClassRecordManager->First();
				while (pConfigRecordInfo)
				{
					SQUICK_SHARE_PTR<IRecord> xRecord = pRecordManager->AddRecord(ident,
						pConfigRecordInfo->GetName(),
						pConfigRecordInfo->GetStartData(),
						pConfigRecordInfo->GetTag(),
						pConfigRecordInfo->GetRows());

					xRecord->SetPublic(pConfigRecordInfo->GetPublic());
					xRecord->SetPrivate(pConfigRecordInfo->GetPrivate());
					xRecord->SetSave(pConfigRecordInfo->GetSave());
					xRecord->SetCache(pConfigRecordInfo->GetCache());
					xRecord->SetUpload(pConfigRecordInfo->GetUpload());

					pObject->AddRecordCallBack(pConfigRecordInfo->GetName(), this, &KernelModule::OnRecordCommonEvent);

					pConfigRecordInfo = pStaticClassRecordManager->Next();
				}
			}
		}

		//no data--main thread
		{
			Vector3 vRelivePos = m_pSceneModule->GetRelivePosition(sceneID, 0);

			pObject->SetPropertyObject(excel::IObject::ID(), ident);
			pObject->SetPropertyString(excel::IObject::ConfigID(), configIndex);
			pObject->SetPropertyString(excel::IObject::ClassName(), className);
			pObject->SetPropertyInt(excel::IObject::SceneID(), sceneID);
			pObject->SetPropertyInt(excel::IObject::GroupID(), groupID);
			pObject->SetPropertyVector3(excel::IObject::Position(), vRelivePos);

			pContainerInfo->AddObjectToGroup(groupID, ident, className == excel::Player::ThisName() ? true : false);

			DoEvent(ident, className, pObject->GetState(), arg);
		}

		//////////////////////////////////////////////////////////////////////////
		//backup thread
		{
			SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
			SQUICK_SHARE_PTR<IPropertyManager> pConfigPropertyManager = m_pElementModule->GetPropertyManager(configIndex);
			SQUICK_SHARE_PTR<IRecordManager> pConfigRecordManager = m_pElementModule->GetRecordManager(configIndex);

			if (pConfigPropertyManager && pConfigRecordManager)
			{
				SQUICK_SHARE_PTR<IProperty> pConfigPropertyInfo = pConfigPropertyManager->First();
				while (nullptr != pConfigPropertyInfo)
				{
					if (pConfigPropertyInfo->Changed())
					{
						pPropertyManager->SetProperty(pConfigPropertyInfo->GetKey(), pConfigPropertyInfo->GetValue());
					}

					pConfigPropertyInfo = pConfigPropertyManager->Next();
				}
			}
		}

		//main thread
		{
			SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
			for (int i = 0; i < arg.GetCount() - 1; i += 2)
			{
				const std::string& propertyName = arg.String(i);
				if (excel::IObject::ConfigID() != propertyName
					&& excel::IObject::ClassName() != propertyName
					&& excel::IObject::SceneID() != propertyName
					&& excel::IObject::ID() != propertyName
					&& excel::IObject::GroupID() != propertyName)
				{
					SQUICK_SHARE_PTR<IProperty> pArgProperty = pPropertyManager->GetElement(propertyName);
					if (pArgProperty)
					{
						switch (pArgProperty->GetType())
						{
						case TDATA_INT:
							pObject->SetPropertyInt(propertyName, arg.Int(i + 1));
							break;
						case TDATA_FLOAT:
							pObject->SetPropertyFloat(propertyName, arg.Float(i + 1));
							break;
						case TDATA_STRING:
							pObject->SetPropertyString(propertyName, arg.String(i + 1));
							break;
						case TDATA_OBJECT:
							pObject->SetPropertyObject(propertyName, arg.Object(i + 1));
							break;
						case TDATA_VECTOR2:
							pObject->SetPropertyVector2(propertyName, arg.Vector2At(i + 1));
							break;
						case TDATA_VECTOR3:
							pObject->SetPropertyVector3(propertyName, arg.Vector3At(i + 1));
							break;
						default:
							break;
						}
					}
				}
			}

			std::ostringstream stream;
			stream << " create object: " << ident.ToString();
			stream << " config_name: " << configIndex;
			stream << " scene_id: " << sceneID;
			stream << " group_id: " << groupID;
			stream << " position: " << pObject->GetPropertyVector3(excel::IObject::Position()).ToString();

			//m_pLogModule->LogInfo(stream);

			pObject->SetState(COE_CREATE_BEFORE_ATTACHDATA);
			DoEvent(ident, className, pObject->GetState(), arg);

		}

		//back up thread
		{
			pObject->SetState(COE_CREATE_LOADDATA);
			DoEvent(ident, className, pObject->GetState(), arg);
		}

		//below are main thread
		{
			pObject->SetState(COE_CREATE_AFTER_ATTACHDATA);
			DoEvent(ident, className, pObject->GetState(), arg);

			pObject->SetState(COE_CREATE_BEFORE_EFFECT);
			DoEvent(ident, className, pObject->GetState(), arg);

			pObject->SetState(COE_CREATE_EFFECTDATA);
			DoEvent(ident, className, pObject->GetState(), arg);

			pObject->SetState(COE_CREATE_AFTER_EFFECT);
			DoEvent(ident, className, pObject->GetState(), arg);

			pObject->SetState(COE_CREATE_READY);
			DoEvent(ident, className, pObject->GetState(), arg);

			pObject->SetState(COE_CREATE_HASDATA);
			DoEvent(ident, className, pObject->GetState(), arg);

			pObject->SetState(COE_CREATE_FINISH);
			DoEvent(ident, className, pObject->GetState(), arg);
		}
	}

	return pObject;
}

bool KernelModule::DestroyObject(const Guid& self)
{
	if (self == mnCurExeObject
		&& !self.IsNull())
	{

		return DestroySelf(self);
	}

	const int sceneID = GetPropertyInt32(self, excel::IObject::SceneID());
	const int groupID = GetPropertyInt32(self, excel::IObject::GroupID());

	SQUICK_SHARE_PTR<SceneInfo> pContainerInfo = m_pSceneModule->GetElement(sceneID);
	if (pContainerInfo)
	{
		const std::string& className = GetPropertyString(self, excel::IObject::ClassName());
		if (className == excel::Player::ThisName())
		{
			m_pSceneModule->LeaveSceneGroup(self);
		}

		DoEvent(self, className, COE_BEFOREDESTROY, DataList::Empty());
		DoEvent(self, className, COE_DESTROY, DataList::Empty());

		if (className != excel::Player::ThisName())
		{
			pContainerInfo->RemoveObjectFromGroup(groupID, self, false);
		}

		RemoveElement(self);
		
		m_pEventModule->RemoveEventCallBack(self);
		m_pScheduleModule->RemoveSchedule(self);

		return true;

	}

	m_pLogModule->LogError(self, "There is no scene " + std::to_string(sceneID), __FUNCTION__, __LINE__);

	return false;
}

bool KernelModule::FindProperty(const Guid& self, const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->FindProperty(propertyName);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

	return false;
}

bool KernelModule::SetPropertyInt(const Guid& self, const std::string& propertyName, const INT64 nValue, const INT64 reason)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->SetPropertyInt(propertyName, nValue, reason);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

	return false;
}

bool KernelModule::SetPropertyFloat(const Guid& self, const std::string& propertyName, const double dValue, const INT64 reason)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->SetPropertyFloat(propertyName, dValue, reason);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

	return false;
}

bool KernelModule::SetPropertyString(const Guid& self, const std::string& propertyName, const std::string& value, const INT64 reason)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->SetPropertyString(propertyName, value, reason);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

	return false;
}

bool KernelModule::SetPropertyObject(const Guid& self, const std::string& propertyName, const Guid& objectValue, const INT64 reason)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->SetPropertyObject(propertyName, objectValue, reason);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

	return false;
}

bool KernelModule::SetPropertyVector2(const Guid& self, const std::string& propertyName, const Vector2& value, const INT64 reason)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->SetPropertyVector2(propertyName, value, reason);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no vector2", __FUNCTION__, __LINE__);

	return false;
}

bool KernelModule::SetPropertyVector3(const Guid& self, const std::string& propertyName, const Vector3& value, const INT64 reason)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->SetPropertyVector3(propertyName, value, reason);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no vector3", __FUNCTION__, __LINE__);

	return false;
}

INT64 KernelModule::GetPropertyInt(const Guid& self, const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetPropertyInt(propertyName);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

	return NULL_INT;
}

int KernelModule::GetPropertyInt32(const Guid& self, const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetPropertyInt32(propertyName);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

	return (int)NULL_INT;
}

double KernelModule::GetPropertyFloat(const Guid& self, const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetPropertyFloat(propertyName);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

	return NULL_FLOAT;
}

const std::string& KernelModule::GetPropertyString(const Guid& self, const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetPropertyString(propertyName);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

	return NULL_STR;
}

const Guid& KernelModule::GetPropertyObject(const Guid& self, const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetPropertyObject(propertyName);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

	return NULL_OBJECT;
}

const Vector2& KernelModule::GetPropertyVector2(const Guid& self, const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetPropertyVector2(propertyName);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no vector2", __FUNCTION__, __LINE__);

	return NULL_VECTOR2;
}

const Vector3& KernelModule::GetPropertyVector3(const Guid& self, const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetPropertyVector3(propertyName);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no vector3", __FUNCTION__, __LINE__);

	return NULL_VECTOR3;
}

SQUICK_SHARE_PTR<IRecord> KernelModule::FindRecord(const Guid& self, const std::string& recordName)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordManager()->GetElement(recordName);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);

	return nullptr;
}

bool KernelModule::ClearRecord(const Guid& self, const std::string& recordName)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = FindRecord(self, recordName);
	if (pRecord)
	{
		return pRecord->Clear();
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no record", __FUNCTION__, __LINE__);

	return false;
}

bool KernelModule::SetRecordInt(const Guid& self, const std::string& recordName, const int row, const int col, const INT64 nValue)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordInt(recordName, row, col, nValue))
		{
			m_pLogModule->LogError(self, recordName + " error for row or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}


	return false;
}

bool KernelModule::SetRecordInt(const Guid& self, const std::string& recordName, const int row, const std::string& colTag, const INT64 value)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordInt(recordName, row, colTag, value))
		{
			m_pLogModule->LogError(self, recordName + " error for row or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

bool KernelModule::SetRecordFloat(const Guid& self, const std::string& recordName, const int row, const int col, const double dwValue)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordFloat(recordName, row, col, dwValue))
		{
			m_pLogModule->LogError(self, recordName + " error SetRecordFloat for row  or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

bool KernelModule::SetRecordFloat(const Guid& self, const std::string& recordName, const int row, const std::string& colTag, const double value)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordFloat(recordName, row, colTag, value))
		{
			m_pLogModule->LogError(self, recordName + " error SetRecordFloat for row  or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

bool KernelModule::SetRecordString(const Guid& self, const std::string& recordName, const int row, const int col, const std::string& value)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordString(recordName, row, col, value))
		{
			m_pLogModule->LogError(self, recordName + " error SetRecordString for row  or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

bool KernelModule::SetRecordString(const Guid& self, const std::string& recordName, const int row, const std::string& colTag, const std::string& value)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordString(recordName, row, colTag, value))
		{
			m_pLogModule->LogError(self, recordName + " error SetRecordObject for row  or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

bool KernelModule::SetRecordObject(const Guid& self, const std::string& recordName, const int row, const int col, const Guid& objectValue)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordObject(recordName, row, col, objectValue))
		{
			m_pLogModule->LogError(self, recordName + " error SetRecordObject for row  or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

bool KernelModule::SetRecordObject(const Guid& self, const std::string& recordName, const int row, const std::string& colTag, const Guid& value)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordObject(recordName, row, colTag, value))
		{
			m_pLogModule->LogError(self, recordName + " error SetRecordObject for row  or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

bool KernelModule::SetRecordVector2(const Guid& self, const std::string& recordName, const int row, const int col, const Vector2& value)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordVector2(recordName, row, col, value))
		{
			m_pLogModule->LogError(self, recordName + " error SetRecordVector2 for row  or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

bool KernelModule::SetRecordVector2(const Guid& self, const std::string& recordName, const int row, const std::string& colTag, const Vector2& value)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordVector2(recordName, row, colTag, value))
		{
			m_pLogModule->LogError(self, recordName + " error SetRecordVector2 for row  or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

bool KernelModule::SetRecordVector3(const Guid& self, const std::string& recordName, const int row, const int col, const Vector3& value)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordVector3(recordName, row, col, value))
		{
			m_pLogModule->LogError(self, recordName + " error SetRecordVector3 for row  or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

bool KernelModule::SetRecordVector3(const Guid& self, const std::string& recordName, const int row, const std::string& colTag, const Vector3& value)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		if (!pObject->SetRecordVector3(recordName, row, colTag, value))
		{
			m_pLogModule->LogError(self, recordName  + " error SetRecordVector3 for row  or col", __FUNCTION__, __LINE__);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
	}

	return false;
}

INT64 KernelModule::GetRecordInt(const Guid& self, const std::string& recordName, const int row, const int col)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordInt(recordName, row, col);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

	return 0;
}

INT64 KernelModule::GetRecordInt(const Guid& self, const std::string& recordName, const int row, const std::string& colTag)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordInt(recordName, row, colTag);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

	return 0;
}

double KernelModule::GetRecordFloat(const Guid& self, const std::string& recordName, const int row, const int col)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordFloat(recordName, row, col);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

	return 0.0;
}

double KernelModule::GetRecordFloat(const Guid& self, const std::string& recordName, const int row, const std::string& colTag)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordFloat(recordName, row, colTag);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

	return 0.0;
}

const std::string& KernelModule::GetRecordString(const Guid& self, const std::string& recordName, const int row, const int col)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordString(recordName, row, col);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

	return NULL_STR;
}

const std::string& KernelModule::GetRecordString(const Guid& self, const std::string& recordName, const int row, const std::string& colTag)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordString(recordName, row, colTag);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

	return NULL_STR;
}

const Guid& KernelModule::GetRecordObject(const Guid& self, const std::string& recordName, const int row, const int col)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordObject(recordName, row, col);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

	return NULL_OBJECT;
}

const Guid& KernelModule::GetRecordObject(const Guid& self, const std::string& recordName, const int row, const std::string& colTag)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordObject(recordName, row, colTag);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

	return NULL_OBJECT;
}

const Vector2& KernelModule::GetRecordVector2(const Guid& self, const std::string& recordName, const int row, const int col)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordVector2(recordName, row, col);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no vector2", __FUNCTION__, __LINE__);

	return NULL_VECTOR2;
}

const Vector2& KernelModule::GetRecordVector2(const Guid& self, const std::string& recordName, const int row, const std::string& colTag)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordVector2(recordName, row, colTag);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no vector2", __FUNCTION__, __LINE__);

	return NULL_VECTOR2;
}

const Vector3& KernelModule::GetRecordVector3(const Guid& self, const std::string& recordName, const int row, const int col)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordVector3(recordName, row, col);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no vector3", __FUNCTION__, __LINE__);

	return NULL_VECTOR3;
}

const Vector3& KernelModule::GetRecordVector3(const Guid& self, const std::string& recordName, const int row, const std::string& colTag)
{
	SQUICK_SHARE_PTR<IObject> pObject = GetElement(self);
	if (pObject)
	{
		return pObject->GetRecordVector3(recordName, row, colTag);
	}

	m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no vector3", __FUNCTION__, __LINE__);

	return NULL_VECTOR3;
}

Guid KernelModule::CreateGUID()
{
	int64_t value = 0;
	uint64_t time = SquickGetTimeMS();


	//value = time << 16;
	value = time * 1000000;


	//value |= nGUIDIndex++;
	value += nGUIDIndex++;

	//if (sequence_ == 0x7FFF)
	if (nGUIDIndex == 999999)
	{
		nGUIDIndex = 0;
	}

	Guid xID;
	xID.nHead64 = pPluginManager->GetAppID();
	xID.nData64 = value;

	return xID;
}

bool KernelModule::CreateScene(const int sceneID)
{
	SQUICK_SHARE_PTR<SceneInfo> pSceneInfo = m_pSceneModule->GetElement(sceneID);
	if (pSceneInfo)
	{
		return false;
	}

	pSceneInfo = SQUICK_SHARE_PTR<SceneInfo>(SQUICK_NEW SceneInfo(sceneID));
	if (pSceneInfo)
	{
		m_pSceneModule->AddElement(sceneID, pSceneInfo);
		RequestGroupScene(sceneID);
		return true;
	}

	return false;
}

bool KernelModule::DestroyScene(const int sceneID)
{
	m_pSceneModule->RemoveElement(sceneID);

	return true;
}

int KernelModule::GetOnLineCount()
{
	int count = 0;
	SQUICK_SHARE_PTR<SceneInfo> pSceneInfo = m_pSceneModule->First();
	while (pSceneInfo)
	{
		SQUICK_SHARE_PTR<SceneGroupInfo> pGroupInfo = pSceneInfo->First();
		while (pGroupInfo)
		{
			count += pGroupInfo->mxPlayerList.Count();
			pGroupInfo = pSceneInfo->Next();
		}

		pSceneInfo = m_pSceneModule->Next();
	}

	return count;
}

int KernelModule::GetMaxOnLineCount()
{
	// test count 5000
	// and it should be define in a xml file

	return 10000;
}

int KernelModule::RequestGroupScene(const int sceneID)
{
	return m_pSceneModule->RequestGroupScene(sceneID);
}

bool KernelModule::ReleaseGroupScene(const int sceneID, const int groupID)
{
	return m_pSceneModule->ReleaseGroupScene(sceneID, groupID);
}

bool KernelModule::ExitGroupScene(const int sceneID, const int groupID)
{
	SQUICK_SHARE_PTR<SceneInfo> pSceneInfo = m_pSceneModule->GetElement(sceneID);
	if (pSceneInfo)
	{
		SQUICK_SHARE_PTR<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(groupID);
		if (pGroupInfo)
		{
			return true;
		}
	}

	return false;
}

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, DataList & list, const Guid & noSelf)
{
	SQUICK_SHARE_PTR<SceneInfo> pSceneInfo = m_pSceneModule->GetElement(sceneID);
	if (pSceneInfo)
	{

		SQUICK_SHARE_PTR<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(groupID);
		if (pGroupInfo)
		{
			Guid ident = Guid();
			SQUICK_SHARE_PTR<int> pRet = pGroupInfo->mxPlayerList.First(ident);
			while (!ident.IsNull())
			{
				if (ident != noSelf)
				{
					list.Add(ident);
				}

				ident = Guid();
				pRet = pGroupInfo->mxPlayerList.Next(ident);
			}

			ident = Guid();
			pRet = pGroupInfo->mxOtherList.First(ident);
			while (!ident.IsNull())
			{
				if (ident != noSelf)
				{
					list.Add(ident);
				}

				ident = Guid();
				pRet = pGroupInfo->mxOtherList.Next(ident);
			}

			return true;
		}
	}

	return false;
}

int KernelModule::GetGroupObjectList(const int sceneID, const int groupID, const bool bPlayer, const Guid & noSelf)
{
	int objectCount = 0;
	SQUICK_SHARE_PTR<SceneInfo> pSceneInfo = m_pSceneModule->GetElement(sceneID);
	if (pSceneInfo)
	{
		SQUICK_SHARE_PTR<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(groupID);
		if (pGroupInfo)
		{
			Guid ident = Guid();
			SQUICK_SHARE_PTR<int> pRet = pGroupInfo->mxPlayerList.First(ident);
			while (!ident.IsNull())
			{
				if (ident != noSelf)
				{
					objectCount++;
				}

				ident = Guid();
				pRet = pGroupInfo->mxPlayerList.Next(ident);
			}

			ident = Guid();
			pRet = pGroupInfo->mxOtherList.First(ident);
			while (!ident.IsNull())
			{
				if (ident != noSelf)
				{
					objectCount++;
				}

				ident = Guid();
				pRet = pGroupInfo->mxOtherList.Next(ident);
			}
		}
	}

	return objectCount;
}

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, DataList& list)
{
	return GetGroupObjectList(sceneID, groupID, list, Guid());
}

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, DataList & list, const bool bPlayer, const Guid & noSelf)
{
	SQUICK_SHARE_PTR<SceneInfo> pSceneInfo = m_pSceneModule->GetElement(sceneID);
	if (pSceneInfo)
	{

		SQUICK_SHARE_PTR<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(groupID);
		if (pGroupInfo)
		{
			if (bPlayer)
			{
				Guid ident = Guid();
				SQUICK_SHARE_PTR<int> pRet = pGroupInfo->mxPlayerList.First(ident);
				while (!ident.IsNull())
				{
					if (ident != noSelf)
					{
						list.Add(ident);
					}

					ident = Guid();
					pRet = pGroupInfo->mxPlayerList.Next(ident);
				}
			}
			else
			{
				Guid ident = Guid();
				SQUICK_SHARE_PTR<int> pRet = pGroupInfo->mxOtherList.First(ident);
				while (!ident.IsNull())
				{
					if (ident != noSelf)
					{
						list.Add(ident);
					}
					ident = Guid();
					pRet = pGroupInfo->mxOtherList.Next(ident);
				}
			}

			return true;
		}
	}
	return false;
}

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, DataList & list, const bool bPlayer)
{
	return GetGroupObjectList(sceneID, groupID, list, bPlayer, Guid());
}

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, const std::string & className, DataList& list, const Guid& noSelf)
{
	DataList xDataList;
	if (GetGroupObjectList(sceneID, groupID, xDataList))
	{
		for (int i = 0; i < xDataList.GetCount(); i++)
		{
			Guid xID = xDataList.Object(i);
			if (xID.IsNull())
			{
				continue;
			}

			if (this->GetPropertyString(xID, excel::IObject::ClassName()) == className
				&& xID != noSelf)
			{
				list.AddObject(xID);
			}
		}

		return true;
	}

	return false;
}

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, const std::string & className, DataList & list)
{
	return GetGroupObjectList(sceneID, groupID, className, list, Guid());
}

bool KernelModule::LogStack()
{
#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
#endif

#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
#endif // SQUICK_PLATFORM

	return true;
}

bool KernelModule::LogInfo(const Guid ident)
{

	SQUICK_SHARE_PTR<IObject> pObject = GetObject(ident);
	if (pObject)
	{
		int sceneID = GetPropertyInt32(ident, excel::IObject::SceneID());
		int groupID = GetPropertyInt32(ident, excel::IObject::GroupID());

		m_pLogModule->LogInfo(ident, "//----------child object list-------- SceneID = " + std::to_string(sceneID));
	}
	else
	{
		m_pLogModule->LogObject(ILogModule::NLL_ERROR_NORMAL, ident, "", __FUNCTION__, __LINE__);
	}

	return true;
}

int KernelModule::OnPropertyCommonEvent(const Guid& self, const std::string& propertyName, const SquickData& oldVar, const SquickData& newVar, const INT64 reason)
{
	Performance performance;

	SQUICK_SHARE_PTR<IObject> xObject = GetElement(self);
	if (xObject)
	{
		if (xObject->GetState() >= CLASS_OBJECT_EVENT::COE_CREATE_HASDATA)
		{
			std::list<PROPERTY_EVENT_FUNCTOR_PTR>::iterator it = mtCommonPropertyCallBackList.begin();
			for (; it != mtCommonPropertyCallBackList.end(); it++)
			{
				PROPERTY_EVENT_FUNCTOR_PTR& pFunPtr = *it;
				PROPERTY_EVENT_FUNCTOR* pFun = pFunPtr.get();
				pFun->operator()(self, propertyName, oldVar, newVar, reason);
			}

			const std::string& className = xObject->GetPropertyString(excel::IObject::ClassName());
			std::map<std::string, std::list<PROPERTY_EVENT_FUNCTOR_PTR>>::iterator itClass = mtClassPropertyCallBackList.find(className);
			if (itClass != mtClassPropertyCallBackList.end())
			{
				std::list<PROPERTY_EVENT_FUNCTOR_PTR>::iterator itList = itClass->second.begin();
				for (; itList != itClass->second.end(); itList++)
				{
					PROPERTY_EVENT_FUNCTOR_PTR& pFunPtr = *itList;
					PROPERTY_EVENT_FUNCTOR* pFun = pFunPtr.get();
					pFun->operator()(self, propertyName, oldVar, newVar, reason);
				}
			}
		}
	}

	if (performance.CheckTimePoint(1))
	{
		std::ostringstream os;
		os << "--------------- performance problem------------------- ";
		os << performance.TimeScope();
		os << "---------- ";
		os << propertyName;
		//m_pLogModule->LogWarning(self, os, __FUNCTION__, __LINE__);
	}


	return 0;
}

SQUICK_SHARE_PTR<IObject> KernelModule::GetObject(const Guid& ident)
{
	return GetElement(ident);
}

int KernelModule::GetObjectByProperty(const int sceneID, const int groupID, const std::string& propertyName, const DataList& valueArg, DataList& list)
{
	DataList varObjectList;
	GetGroupObjectList(sceneID, groupID, varObjectList);

	int nWorldCount = varObjectList.GetCount();
	for (int i = 0; i < nWorldCount; i++)
	{
		Guid ident = varObjectList.Object(i);
		if (this->FindProperty(ident, propertyName))
		{
			DATA_TYPE eType = valueArg.Type(0);
			switch (eType)
			{
			case TDATA_INT:
			{
				int64_t nValue = GetPropertyInt(ident, propertyName.c_str());
				if (valueArg.Int(0) == nValue)
				{
					list.Add(ident);
				}
			}
			break;
			case TDATA_STRING:
			{
				std::string strValue = GetPropertyString(ident, propertyName.c_str());
				std::string strCompareValue = valueArg.String(0);
				if (strValue == strCompareValue)
				{
					list.Add(ident);
				}
			}
			break;
			case TDATA_OBJECT:
			{
				Guid identObject = GetPropertyObject(ident, propertyName.c_str());
				if (valueArg.Object(0) == identObject)
				{
					list.Add(ident);
				}
			}
			break;
			default:
				break;
			}
		}
	}

	return list.GetCount();
}

bool KernelModule::ExistScene(const int sceneID)
{
	SQUICK_SHARE_PTR<SceneInfo> pSceneInfo = m_pSceneModule->GetElement(sceneID);
	if (pSceneInfo)
	{
		return true;
	}

	return false;
}

bool KernelModule::ExistObject(const Guid & ident)
{
	return ExistElement(ident);
}

bool KernelModule::ObjectReady(const Guid& ident)
{
	auto gameObject = GetElement(ident);
	if (gameObject)
	{
		return gameObject->ObjectReady();
	}

	return false;
}

bool KernelModule::ExistObject(const Guid & ident, const int sceneID, const int groupID)
{
	SQUICK_SHARE_PTR<SceneInfo> pSceneInfo = m_pSceneModule->GetElement(sceneID);
	if (!pSceneInfo)
	{
		return false;
	}

	return pSceneInfo->ExistObjectInGroup(groupID, ident);
}

bool KernelModule::DestroySelf(const Guid& self)
{
	mtDeleteSelfList.push_back(self);
	return true;
}

int KernelModule::OnRecordCommonEvent(const Guid& self, const RECORD_EVENT_DATA& eventData, const SquickData& oldVar, const SquickData& newVar)
{
	Performance performance;

	SQUICK_SHARE_PTR<IObject> xObject = GetElement(self);
	if (xObject)
	{
		if (xObject->GetState() >= CLASS_OBJECT_EVENT::COE_CREATE_HASDATA)
		{
			std::list<RECORD_EVENT_FUNCTOR_PTR>::iterator it = mtCommonRecordCallBackList.begin();
			for (; it != mtCommonRecordCallBackList.end(); it++)
			{
				RECORD_EVENT_FUNCTOR_PTR& pFunPtr = *it;
				RECORD_EVENT_FUNCTOR* pFun = pFunPtr.get();
				pFun->operator()(self, eventData, oldVar, newVar);
			}
		}

		const std::string& className = xObject->GetPropertyString(excel::IObject::ClassName());
		std::map<std::string, std::list<RECORD_EVENT_FUNCTOR_PTR>>::iterator itClass = mtClassRecordCallBackList.find(className);
		if (itClass != mtClassRecordCallBackList.end())
		{
			std::list<RECORD_EVENT_FUNCTOR_PTR>::iterator itList = itClass->second.begin();
			for (; itList != itClass->second.end(); itList++)
			{
				RECORD_EVENT_FUNCTOR_PTR& pFunPtr = *itList;
				RECORD_EVENT_FUNCTOR* pFun = pFunPtr.get();
				pFun->operator()(self, eventData, oldVar, newVar);
			}
		}
	}

	if (performance.CheckTimePoint(1))
	{
		std::ostringstream os;
		os << "--------------- performance problem------------------- ";
		os << performance.TimeScope();
		os << "---------- ";
		os << eventData.recordName;
		os << " event type " << eventData.nOpType;
		//m_pLogModule->LogWarning(self, os, __FUNCTION__, __LINE__);
	}

	return 0;
}

int KernelModule::OnClassCommonEvent(const Guid& self, const std::string& className, const CLASS_OBJECT_EVENT classEvent, const DataList& var)
{
	Performance performance;

	std::list<CLASS_EVENT_FUNCTOR_PTR>::iterator it = mtCommonClassCallBackList.begin();
	for (; it != mtCommonClassCallBackList.end(); it++)
	{
		CLASS_EVENT_FUNCTOR_PTR& pFunPtr = *it;
		CLASS_EVENT_FUNCTOR* pFun = pFunPtr.get();
		pFun->operator()(self, className, classEvent, var);
	}

	if (performance.CheckTimePoint(1))
	{
		std::ostringstream os;
		os << "--------------- performance problem------------------- ";
		os << performance.TimeScope();
		os << "---------- ";
		os << className;
		os << " event type " << classEvent;
		//m_pLogModule->LogWarning(self, os, __FUNCTION__, __LINE__);
	}

	return 0;
}

bool KernelModule::RegisterCommonClassEvent(const CLASS_EVENT_FUNCTOR_PTR& cb)
{
	mtCommonClassCallBackList.push_back(cb);
	return true;
}

bool KernelModule::RegisterCommonPropertyEvent(const PROPERTY_EVENT_FUNCTOR_PTR& cb)
{
	mtCommonPropertyCallBackList.push_back(cb);
	return true;
}

bool KernelModule::RegisterCommonRecordEvent(const RECORD_EVENT_FUNCTOR_PTR& cb)
{
	mtCommonRecordCallBackList.push_back(cb);
	return true;
}

bool KernelModule::RegisterClassPropertyEvent(const std::string & className, const PROPERTY_EVENT_FUNCTOR_PTR & cb)
{
	if (mtClassPropertyCallBackList.find(className) == mtClassPropertyCallBackList.end())
	{
		std::list<PROPERTY_EVENT_FUNCTOR_PTR> xList;
		xList.push_back(cb);

		mtClassPropertyCallBackList.insert(std::map< std::string, std::list<PROPERTY_EVENT_FUNCTOR_PTR>>::value_type(className, xList));

		return true;
	}


	std::map< std::string, std::list<PROPERTY_EVENT_FUNCTOR_PTR>>::iterator it = mtClassPropertyCallBackList.find(className);
	it->second.push_back(cb);


	return false;
}

bool KernelModule::RegisterClassRecordEvent(const std::string & className, const RECORD_EVENT_FUNCTOR_PTR & cb)
{
	if (mtClassRecordCallBackList.find(className) == mtClassRecordCallBackList.end())
	{
		std::list<RECORD_EVENT_FUNCTOR_PTR> xList;
		xList.push_back(cb);

		mtClassRecordCallBackList.insert(std::map< std::string, std::list<RECORD_EVENT_FUNCTOR_PTR>>::value_type(className, xList));

		return true;
	}


	std::map< std::string, std::list<RECORD_EVENT_FUNCTOR_PTR>>::iterator it = mtClassRecordCallBackList.find(className);
	it->second.push_back(cb);

	return true;
}

bool KernelModule::LogSelfInfo(const Guid ident)
{

	return false;
}

bool KernelModule::AfterStart()
{
	SQUICK_SHARE_PTR<IClass> pClass = m_pClassModule->First();
	while (pClass)
	{
		IKernelModule::AddClassCallBack(pClass->GetClassName(), this, &KernelModule::OnClassCommonEvent);

		pClass = m_pClassModule->Next();
	}

	return true;
}

bool KernelModule::DestroyAll()
{
	SQUICK_SHARE_PTR<IObject> pObject = First();
	while (pObject)
	{
		mtDeleteSelfList.push_back(pObject->Self());

		pObject = Next();
	}


	Update();

	m_pSceneModule->ClearAll();

	return true;
}

bool KernelModule::BeforeDestory()
{
	DestroyAll();

	mvRandom.clear();
	mtCommonClassCallBackList.clear();
	mtCommonPropertyCallBackList.clear();
	mtCommonRecordCallBackList.clear();

	mtClassPropertyCallBackList.clear();
	mtClassRecordCallBackList.clear();

	return true;
}

int KernelModule::Random(int nStart, int nEnd)
{
	if (++mxRandomItor == mvRandom.cend())
	{
		mxRandomItor = mvRandom.cbegin();
	}

	return static_cast<int>((nEnd - nStart) * *mxRandomItor) + nStart;
}

float KernelModule::Random()
{
	if (++mxRandomItor == mvRandom.cend())
	{
		mxRandomItor = mvRandom.cbegin();
	}

	return *mxRandomItor;
}

bool KernelModule::AddClassCallBack(const std::string& className, const CLASS_EVENT_FUNCTOR_PTR& cb)
{
	return m_pClassModule->AddClassCallBack(className, cb);
}

void KernelModule::ProcessMemFree()
{
	if (nLastTime + 30 > pPluginManager->GetNowTime())
	{
		return;
	}

	nLastTime = pPluginManager->GetNowTime();

	std::string info;

	// SQUICK_WILL_DO
	//MemoryCounter::PrintMemoryInfo(info);

	//m_pLogModule->LogInfo(info, __FUNCTION__, __LINE__);
	
	//MemManager::GetSingletonPtr()->FreeMem();
	//MallocExtension::instance()->ReleaseFreeMemory();
}

bool KernelModule::DoEvent(const Guid& self, const std::string& className, CLASS_OBJECT_EVENT eEvent, const DataList& valueList)
{
	return m_pClassModule->DoEvent(self, className, eEvent, valueList);
}