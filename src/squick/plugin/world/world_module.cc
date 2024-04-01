
#include "world_module.h"
#include "scene_module.h"
#include <squick/core/guid.h>
#include <squick/core/mem_manager.h>
#include <squick/core/memory_counter.h>
#include <squick/core/object.h>
#include <squick/core/performance.h>
#include <squick/core/property_manager.h>
#include <squick/core/record.h>
#include <squick/core/record_manager.h>
#include <struct/excel.h>

KernelModule::KernelModule(IPluginManager *p) {
    is_update_ = true;
    nGUIDIndex = 0;
    nLastTime = 0;

    pm_ = p;

    nLastTime = pm_->GetNowTime();
    StartRandom();
}

KernelModule::~KernelModule() { ClearAll(); }

void KernelModule::StartRandom() {
    mvRandom.clear();

    constexpr int nRandomMax = 100000;
    mvRandom.reserve(nRandomMax);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1.0f);

    for (int i = 0; i < nRandomMax; i++) {
        mvRandom.emplace_back((float)dis(gen));
    }

    mxRandomItor = mvRandom.cbegin();
}

bool KernelModule::Start() {
    mtDeleteSelfList.clear();

    m_scene_ = pm_->FindModule<ISceneModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_schedule_ = pm_->FindModule<IScheduleModule>();
    m_event_ = pm_->FindModule<IEventModule>();
    m_pCellModule = pm_->FindModule<ICellModule>();
    m_thread_pool_ = pm_->FindModule<IThreadPoolModule>();
    return true;
}

bool KernelModule::Destory() { return true; }

bool KernelModule::Update() {
    ProcessMemFree();

    mnCurExeObject.nHead64 = 0;
    mnCurExeObject.nData64 = 0;

    if (mtDeleteSelfList.size() > 0) {
        std::list<Guid>::iterator it = mtDeleteSelfList.begin();
        for (; it != mtDeleteSelfList.end(); it++) {
            DestroyObject(*it);
        }
        mtDeleteSelfList.clear();
    }

    return true;
}

std::shared_ptr<IObject> KernelModule::CreateObject(const Guid &self, const int sceneID, const int groupID, const std::string &className,
                                                    const std::string &configIndex, const DataList &arg) {
    std::shared_ptr<IObject> pObject;
    Guid ident = self;

    std::shared_ptr<SceneInfo> pContainerInfo = m_scene_->GetElement(sceneID);
    if (!pContainerInfo) {
        m_log_->LogError(Guid(0, sceneID), "There is no scene " + std::to_string(sceneID), __FUNCTION__, __LINE__);
        return pObject;
    }

    if (!pContainerInfo->GetElement(groupID)) {
        m_log_->LogError("There is no group " + std::to_string(groupID), __FUNCTION__, __LINE__);
        return pObject;
    }

    //  if (!m_element_->ExistElement(configIndex))
    //  {
    //      m_log_->LogError(Guid(0, sceneID), "There is no group", groupID, __FUNCTION__, __LINE__);
    //      return pObject;
    //  }

    if (ident.IsNull()) {
        ident = CreateGUID();
    }

    if (GetElement(ident)) {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, ident, "The object has Exists", __FUNCTION__, __LINE__);
        return pObject;
    }

    pObject = std::shared_ptr<IObject>(new Object(ident, pm_));
    AddElement(ident, pObject);

    if (pm_->UsingBackThread()) {
        m_thread_pool_->DoAsyncTask(
            Guid(), "",
            [=](ThreadTask &task) -> void {
                // backup thread for async task
                {
                    std::shared_ptr<IPropertyManager> pStaticClassPropertyManager = m_class_->GetThreadClassModule()->GetClassPropertyManager(className);
                    std::shared_ptr<IRecordManager> pStaticClassRecordManager = m_class_->GetThreadClassModule()->GetClassRecordManager(className);
                    if (pStaticClassPropertyManager && pStaticClassRecordManager) {
                        std::shared_ptr<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
                        std::shared_ptr<IRecordManager> pRecordManager = pObject->GetRecordManager();

                        std::shared_ptr<IProperty> pStaticConfigPropertyInfo = pStaticClassPropertyManager->First();
                        while (pStaticConfigPropertyInfo) {
                            std::shared_ptr<IProperty> xProperty =
                                pPropertyManager->AddProperty(ident, pStaticConfigPropertyInfo->GetKey(), pStaticConfigPropertyInfo->GetType());

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

                        std::shared_ptr<IRecord> pConfigRecordInfo = pStaticClassRecordManager->First();
                        while (pConfigRecordInfo) {
                            std::shared_ptr<IRecord> xRecord = pRecordManager->AddRecord(ident, pConfigRecordInfo->GetName(), pConfigRecordInfo->GetStartData(),
                                                                                         pConfigRecordInfo->GetTag(), pConfigRecordInfo->GetRows());

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
            [=](ThreadTask &task) -> void {
                // no data--main thread
                {
                    Vector3 vRelivePos = m_scene_->GetRelivePosition(sceneID, 0);

                    pObject->SetPropertyString(excel::IObject::ConfigID(), configIndex);
                    pObject->SetPropertyString(excel::IObject::ClassName(), className);
                    pObject->SetPropertyInt(excel::IObject::SceneID(), sceneID);
                    pObject->SetPropertyInt(excel::IObject::GroupID(), groupID);
                    pObject->SetPropertyVector3(excel::IObject::Position(), vRelivePos);

                    pContainerInfo->AddObjectToGroup(groupID, ident, className == excel::Player::ThisName() ? true : false);

                    DoEvent(ident, className, pObject->GetState(), arg);
                }

                m_thread_pool_->DoAsyncTask(
                    Guid(), "",
                    [=](ThreadTask &task) -> void {
                        // backup thread
                        {
                            std::shared_ptr<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
                            std::shared_ptr<IPropertyManager> pConfigPropertyManager = m_element_->GetThreadElementModule()->GetPropertyManager(configIndex);
                            std::shared_ptr<IRecordManager> pConfigRecordManager = m_element_->GetThreadElementModule()->GetRecordManager(configIndex);

                            if (pConfigPropertyManager && pConfigRecordManager) {
                                std::shared_ptr<IProperty> pConfigPropertyInfo = pConfigPropertyManager->First();
                                while (nullptr != pConfigPropertyInfo) {
                                    if (pConfigPropertyInfo->Changed()) {
                                        pPropertyManager->SetProperty(pConfigPropertyInfo->GetKey(), pConfigPropertyInfo->GetValue());
                                    }

                                    pConfigPropertyInfo = pConfigPropertyManager->Next();
                                }
                            }
                        }
                    },
                    [=](ThreadTask &task) -> void {
                        // main thread
                        {
                            std::shared_ptr<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
                            for (int i = 0; i < arg.GetCount() - 1; i += 2) {
                                const std::string &propertyName = arg.String(i);
                                if (excel::IObject::ConfigID() != propertyName && excel::IObject::ClassName() != propertyName &&
                                    excel::IObject::SceneID() != propertyName && excel::IObject::ID() != propertyName &&
                                    excel::IObject::GroupID() != propertyName) {
                                    std::shared_ptr<IProperty> pArgProperty = pPropertyManager->GetElement(propertyName);
                                    if (pArgProperty) {
                                        switch (pArgProperty->GetType()) {
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

                            m_log_->LogInfo(stream);

                            pObject->SetState(COE_CREATE_BEFORE_ATTACHDATA);
                            DoEvent(ident, className, pObject->GetState(), arg);
                        }

                        m_thread_pool_->DoAsyncTask(
                            Guid(), "",
                            [=](ThreadTask &task) -> void {
                                // back up thread
                                {
                                    pObject->SetState(COE_CREATE_LOADDATA);
                                    DoEvent(ident, className, pObject->GetState(), arg);
                                }
                            },
                            [=](ThreadTask &task) -> void {
                                // below are main thread
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

    } else {
        // backup thread for async task
        {
            std::shared_ptr<IPropertyManager> pStaticClassPropertyManager = m_class_->GetClassPropertyManager(className);
            std::shared_ptr<IRecordManager> pStaticClassRecordManager = m_class_->GetClassRecordManager(className);
            if (pStaticClassPropertyManager && pStaticClassRecordManager) {
                std::shared_ptr<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
                std::shared_ptr<IRecordManager> pRecordManager = pObject->GetRecordManager();

                std::shared_ptr<IProperty> pStaticConfigPropertyInfo = pStaticClassPropertyManager->First();
                while (pStaticConfigPropertyInfo) {
                    std::shared_ptr<IProperty> xProperty =
                        pPropertyManager->AddProperty(ident, pStaticConfigPropertyInfo->GetKey(), pStaticConfigPropertyInfo->GetType());

                    xProperty->SetPublic(pStaticConfigPropertyInfo->GetPublic());
                    xProperty->SetPrivate(pStaticConfigPropertyInfo->GetPrivate());
                    xProperty->SetSave(pStaticConfigPropertyInfo->GetSave());
                    xProperty->SetCache(pStaticConfigPropertyInfo->GetCache());
                    xProperty->SetRef(pStaticConfigPropertyInfo->GetRef());
                    xProperty->SetUpload(pStaticConfigPropertyInfo->GetUpload());

                    pObject->AddPropertyCallBack(pStaticConfigPropertyInfo->GetKey(), this, &KernelModule::OnPropertyCommonEvent);

                    pStaticConfigPropertyInfo = pStaticClassPropertyManager->Next();
                }

                std::shared_ptr<IRecord> pConfigRecordInfo = pStaticClassRecordManager->First();
                while (pConfigRecordInfo) {
                    std::shared_ptr<IRecord> xRecord = pRecordManager->AddRecord(ident, pConfigRecordInfo->GetName(), pConfigRecordInfo->GetStartData(),
                                                                                 pConfigRecordInfo->GetTag(), pConfigRecordInfo->GetRows());

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

        // no data--main thread
        {
            Vector3 vRelivePos = m_scene_->GetRelivePosition(sceneID, 0);

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
        // backup thread
        {
            std::shared_ptr<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
            std::shared_ptr<IPropertyManager> pConfigPropertyManager = m_element_->GetPropertyManager(configIndex);
            std::shared_ptr<IRecordManager> pConfigRecordManager = m_element_->GetRecordManager(configIndex);

            if (pConfigPropertyManager && pConfigRecordManager) {
                std::shared_ptr<IProperty> pConfigPropertyInfo = pConfigPropertyManager->First();
                while (nullptr != pConfigPropertyInfo) {
                    if (pConfigPropertyInfo->Changed()) {
                        pPropertyManager->SetProperty(pConfigPropertyInfo->GetKey(), pConfigPropertyInfo->GetValue());
                    }

                    pConfigPropertyInfo = pConfigPropertyManager->Next();
                }
            }
        }

        // main thread
        {
            std::shared_ptr<IPropertyManager> pPropertyManager = pObject->GetPropertyManager();
            for (int i = 0; i < arg.GetCount() - 1; i += 2) {
                const std::string &propertyName = arg.String(i);
                if (excel::IObject::ConfigID() != propertyName && excel::IObject::ClassName() != propertyName && excel::IObject::SceneID() != propertyName &&
                    excel::IObject::ID() != propertyName && excel::IObject::GroupID() != propertyName) {
                    std::shared_ptr<IProperty> pArgProperty = pPropertyManager->GetElement(propertyName);
                    if (pArgProperty) {
                        switch (pArgProperty->GetType()) {
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

            // m_log_->LogInfo(stream);

            pObject->SetState(COE_CREATE_BEFORE_ATTACHDATA);
            DoEvent(ident, className, pObject->GetState(), arg);
        }

        // back up thread
        {
            pObject->SetState(COE_CREATE_LOADDATA);
            DoEvent(ident, className, pObject->GetState(), arg);
        }

        // below are main thread
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

bool KernelModule::DestroyObject(const Guid &self) {
    if (self == mnCurExeObject && !self.IsNull()) {

        return DestroySelf(self);
    }

    const int sceneID = GetPropertyInt32(self, excel::IObject::SceneID());
    const int groupID = GetPropertyInt32(self, excel::IObject::GroupID());

    std::shared_ptr<SceneInfo> pContainerInfo = m_scene_->GetElement(sceneID);
    if (pContainerInfo) {
        const std::string &className = GetPropertyString(self, excel::IObject::ClassName());
        if (className == excel::Player::ThisName()) {
            m_scene_->LeaveSceneGroup(self);
        }

        DoEvent(self, className, COE_BEFOREDESTROY, DataList::Empty());
        DoEvent(self, className, COE_DESTROY, DataList::Empty());

        if (className != excel::Player::ThisName()) {
            pContainerInfo->RemoveObjectFromGroup(groupID, self, false);
        }

        RemoveElement(self);

        m_event_->RemoveEventCallBack(self);
        m_schedule_->RemoveSchedule(self);

        return true;
    }

    m_log_->LogError(self, "There is no scene " + std::to_string(sceneID), __FUNCTION__, __LINE__);

    return false;
}

bool KernelModule::FindProperty(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->FindProperty(propertyName);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

    return false;
}

bool KernelModule::SetPropertyInt(const Guid &self, const std::string &propertyName, const INT64 nValue, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyInt(propertyName, nValue, reason);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

    return false;
}

bool KernelModule::SetPropertyFloat(const Guid &self, const std::string &propertyName, const double dValue, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyFloat(propertyName, dValue, reason);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

    return false;
}

bool KernelModule::SetPropertyString(const Guid &self, const std::string &propertyName, const std::string &value, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyString(propertyName, value, reason);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

    return false;
}

bool KernelModule::SetPropertyObject(const Guid &self, const std::string &propertyName, const Guid &objectValue, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyObject(propertyName, objectValue, reason);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

    return false;
}

bool KernelModule::SetPropertyVector2(const Guid &self, const std::string &propertyName, const Vector2 &value, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyVector2(propertyName, value, reason);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no vector2", __FUNCTION__, __LINE__);

    return false;
}

bool KernelModule::SetPropertyVector3(const Guid &self, const std::string &propertyName, const Vector3 &value, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyVector3(propertyName, value, reason);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no vector3", __FUNCTION__, __LINE__);

    return false;
}

INT64 KernelModule::GetPropertyInt(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyInt(propertyName);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

    return NULL_INT;
}

int KernelModule::GetPropertyInt32(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyInt32(propertyName);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

    return (int)NULL_INT;
}

double KernelModule::GetPropertyFloat(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyFloat(propertyName);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

    return NULL_FLOAT;
}

const std::string &KernelModule::GetPropertyString(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyString(propertyName);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

    return NULL_STR;
}

const Guid &KernelModule::GetPropertyObject(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyObject(propertyName);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no object", __FUNCTION__, __LINE__);

    return NULL_OBJECT;
}

const Vector2 &KernelModule::GetPropertyVector2(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyVector2(propertyName);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no vector2", __FUNCTION__, __LINE__);

    return NULL_VECTOR2;
}

const Vector3 &KernelModule::GetPropertyVector3(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyVector3(propertyName);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, propertyName + "| There is no vector3", __FUNCTION__, __LINE__);

    return NULL_VECTOR3;
}

std::shared_ptr<IRecord> KernelModule::FindRecord(const Guid &self, const std::string &recordName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordManager()->GetElement(recordName);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);

    return nullptr;
}

bool KernelModule::ClearRecord(const Guid &self, const std::string &recordName) {
    std::shared_ptr<IRecord> pRecord = FindRecord(self, recordName);
    if (pRecord) {
        return pRecord->Clear();
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no record", __FUNCTION__, __LINE__);

    return false;
}

bool KernelModule::SetRecordInt(const Guid &self, const std::string &recordName, const int row, const int col, const INT64 nValue) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordInt(recordName, row, col, nValue)) {
            m_log_->LogError(self, recordName + " error for row or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordInt(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const INT64 value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordInt(recordName, row, colTag, value)) {
            m_log_->LogError(self, recordName + " error for row or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordFloat(const Guid &self, const std::string &recordName, const int row, const int col, const double dwValue) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordFloat(recordName, row, col, dwValue)) {
            m_log_->LogError(self, recordName + " error SetRecordFloat for row  or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordFloat(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const double value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordFloat(recordName, row, colTag, value)) {
            m_log_->LogError(self, recordName + " error SetRecordFloat for row  or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordString(const Guid &self, const std::string &recordName, const int row, const int col, const std::string &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordString(recordName, row, col, value)) {
            m_log_->LogError(self, recordName + " error SetRecordString for row  or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordString(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const std::string &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordString(recordName, row, colTag, value)) {
            m_log_->LogError(self, recordName + " error SetRecordObject for row  or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordObject(const Guid &self, const std::string &recordName, const int row, const int col, const Guid &objectValue) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordObject(recordName, row, col, objectValue)) {
            m_log_->LogError(self, recordName + " error SetRecordObject for row  or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordObject(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Guid &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordObject(recordName, row, colTag, value)) {
            m_log_->LogError(self, recordName + " error SetRecordObject for row  or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordVector2(const Guid &self, const std::string &recordName, const int row, const int col, const Vector2 &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordVector2(recordName, row, col, value)) {
            m_log_->LogError(self, recordName + " error SetRecordVector2 for row  or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordVector2(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Vector2 &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordVector2(recordName, row, colTag, value)) {
            m_log_->LogError(self, recordName + " error SetRecordVector2 for row  or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordVector3(const Guid &self, const std::string &recordName, const int row, const int col, const Vector3 &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordVector3(recordName, row, col, value)) {
            m_log_->LogError(self, recordName + " error SetRecordVector3 for row  or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

bool KernelModule::SetRecordVector3(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Vector3 &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordVector3(recordName, row, colTag, value)) {
            m_log_->LogError(self, recordName + " error SetRecordVector3 for row  or col", __FUNCTION__, __LINE__);
        } else {
            return true;
        }
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, recordName + "| There is no object", __FUNCTION__, __LINE__);
    }

    return false;
}

INT64 KernelModule::GetRecordInt(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordInt(recordName, row, col);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

    return 0;
}

INT64 KernelModule::GetRecordInt(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordInt(recordName, row, colTag);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

    return 0;
}

double KernelModule::GetRecordFloat(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordFloat(recordName, row, col);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

    return 0.0;
}

double KernelModule::GetRecordFloat(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordFloat(recordName, row, colTag);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

    return 0.0;
}

const std::string &KernelModule::GetRecordString(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordString(recordName, row, col);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

    return NULL_STR;
}

const std::string &KernelModule::GetRecordString(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordString(recordName, row, colTag);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

    return NULL_STR;
}

const Guid &KernelModule::GetRecordObject(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordObject(recordName, row, col);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

    return NULL_OBJECT;
}

const Guid &KernelModule::GetRecordObject(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordObject(recordName, row, colTag);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

    return NULL_OBJECT;
}

const Vector2 &KernelModule::GetRecordVector2(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordVector2(recordName, row, col);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no vector2", __FUNCTION__, __LINE__);

    return NULL_VECTOR2;
}

const Vector2 &KernelModule::GetRecordVector2(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordVector2(recordName, row, colTag);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no vector2", __FUNCTION__, __LINE__);

    return NULL_VECTOR2;
}

const Vector3 &KernelModule::GetRecordVector3(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordVector3(recordName, row, col);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no vector3", __FUNCTION__, __LINE__);

    return NULL_VECTOR3;
}

const Vector3 &KernelModule::GetRecordVector3(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordVector3(recordName, row, colTag);
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no vector3", __FUNCTION__, __LINE__);

    return NULL_VECTOR3;
}

Guid KernelModule::CreateGUID() {
    int64_t value = 0;
    uint64_t time = SquickGetTimeMS();

    // value = time << 16;
    value = time * 1000000;

    // value |= nGUIDIndex++;
    value += nGUIDIndex++;

    // if (sequence_ == 0x7FFF)
    if (nGUIDIndex == 999999) {
        nGUIDIndex = 0;
    }

    Guid xID;
    xID.nHead64 = pm_->GetAppID();
    xID.nData64 = value;

    return xID;
}

Guid KernelModule::CreatePlayerGUID() {
    int64_t value = 0;
    uint64_t time = SquickGetTimeMS();

    // value = time << 16;
    value = time * 1000000;

    // value |= nGUIDIndex++;
    value += nGUIDIndex++;

    // if (sequence_ == 0x7FFF)
    if (nGUIDIndex == 999999) {
        nGUIDIndex = 0;
    }

    Guid xID;
    xID.nHead64 = (pm_->GetArea() << 32) + pm_->GetAppID();
    xID.nData64 = value;

    return xID;
}


bool KernelModule::CreateScene(const int sceneID) {
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->GetElement(sceneID);
    if (pSceneInfo) {
        return false;
    }

    pSceneInfo = std::shared_ptr<SceneInfo>(new SceneInfo(sceneID));
    if (pSceneInfo) {
        m_scene_->AddElement(sceneID, pSceneInfo);
        RequestGroupScene(sceneID);
        return true;
    }

    return false;
}

bool KernelModule::DestroyScene(const int sceneID) {
    m_scene_->RemoveElement(sceneID);

    return true;
}

int KernelModule::GetOnLineCount() {
    int count = 0;
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->First();
    while (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->First();
        while (pGroupInfo) {
            count += pGroupInfo->mxPlayerList.Count();
            pGroupInfo = pSceneInfo->Next();
        }

        pSceneInfo = m_scene_->Next();
    }

    return count;
}

int KernelModule::GetMaxOnLineCount() {
    // test count 5000
    // and it should be define in a xml file

    return 10000;
}

int KernelModule::RequestGroupScene(const int sceneID) { return m_scene_->RequestGroupScene(sceneID); }

bool KernelModule::ReleaseGroupScene(const int sceneID, const int groupID) { return m_scene_->ReleaseGroupScene(sceneID, groupID); }

bool KernelModule::ExitGroupScene(const int sceneID, const int groupID) {
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->GetElement(sceneID);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(groupID);
        if (pGroupInfo) {
            return true;
        }
    }

    return false;
}

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, DataList &list, const Guid &noSelf) {
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->GetElement(sceneID);
    if (pSceneInfo) {

        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(groupID);
        if (pGroupInfo) {
            Guid ident = Guid();
            std::shared_ptr<int> pRet = pGroupInfo->mxPlayerList.First(ident);
            while (!ident.IsNull()) {
                if (ident != noSelf) {
                    list.Add(ident);
                }

                ident = Guid();
                pRet = pGroupInfo->mxPlayerList.Next(ident);
            }

            ident = Guid();
            pRet = pGroupInfo->mxOtherList.First(ident);
            while (!ident.IsNull()) {
                if (ident != noSelf) {
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

int KernelModule::GetGroupObjectList(const int sceneID, const int groupID, const bool bPlayer, const Guid &noSelf) {
    int objectCount = 0;
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->GetElement(sceneID);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(groupID);
        if (pGroupInfo) {
            Guid ident = Guid();
            std::shared_ptr<int> pRet = pGroupInfo->mxPlayerList.First(ident);
            while (!ident.IsNull()) {
                if (ident != noSelf) {
                    objectCount++;
                }

                ident = Guid();
                pRet = pGroupInfo->mxPlayerList.Next(ident);
            }

            ident = Guid();
            pRet = pGroupInfo->mxOtherList.First(ident);
            while (!ident.IsNull()) {
                if (ident != noSelf) {
                    objectCount++;
                }

                ident = Guid();
                pRet = pGroupInfo->mxOtherList.Next(ident);
            }
        }
    }

    return objectCount;
}

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, DataList &list) { return GetGroupObjectList(sceneID, groupID, list, Guid()); }

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, DataList &list, const bool bPlayer, const Guid &noSelf) {
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->GetElement(sceneID);
    if (pSceneInfo) {

        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(groupID);
        if (pGroupInfo) {
            if (bPlayer) {
                Guid ident = Guid();
                std::shared_ptr<int> pRet = pGroupInfo->mxPlayerList.First(ident);
                while (!ident.IsNull()) {
                    if (ident != noSelf) {
                        list.Add(ident);
                    }

                    ident = Guid();
                    pRet = pGroupInfo->mxPlayerList.Next(ident);
                }
            } else {
                Guid ident = Guid();
                std::shared_ptr<int> pRet = pGroupInfo->mxOtherList.First(ident);
                while (!ident.IsNull()) {
                    if (ident != noSelf) {
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

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, DataList &list, const bool bPlayer) {
    return GetGroupObjectList(sceneID, groupID, list, bPlayer, Guid());
}

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, const std::string &className, DataList &list, const Guid &noSelf) {
    DataList xDataList;
    if (GetGroupObjectList(sceneID, groupID, xDataList)) {
        for (int i = 0; i < xDataList.GetCount(); i++) {
            Guid xID = xDataList.Object(i);
            if (xID.IsNull()) {
                continue;
            }

            if (this->GetPropertyString(xID, excel::IObject::ClassName()) == className && xID != noSelf) {
                list.AddObject(xID);
            }
        }

        return true;
    }

    return false;
}

bool KernelModule::GetGroupObjectList(const int sceneID, const int groupID, const std::string &className, DataList &list) {
    return GetGroupObjectList(sceneID, groupID, className, list, Guid());
}

bool KernelModule::LogStack() {
#if PLATFORM == PLATFORM_WIN
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
#endif

#if PLATFORM == PLATFORM_WIN
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
#endif // PLATFORM

    return true;
}

bool KernelModule::LogInfo(const Guid ident) {

    std::shared_ptr<IObject> pObject = GetObject(ident);
    if (pObject) {
        int sceneID = GetPropertyInt32(ident, excel::IObject::SceneID());
        int groupID = GetPropertyInt32(ident, excel::IObject::GroupID());

        m_log_->LogInfo(ident, "//----------child object list-------- SceneID = " + std::to_string(sceneID));
    } else {
        m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, ident, "", __FUNCTION__, __LINE__);
    }

    return true;
}

int KernelModule::OnPropertyCommonEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar,
                                        const INT64 reason) {
    Performance performance;

    std::shared_ptr<IObject> xObject = GetElement(self);
    if (xObject) {
        if (xObject->GetState() >= CLASS_OBJECT_EVENT::COE_CREATE_HASDATA) {
            std::list<PROPERTY_EVENT_FUNCTOR_PTR>::iterator it = mtCommonPropertyCallBackList.begin();
            for (; it != mtCommonPropertyCallBackList.end(); it++) {
                PROPERTY_EVENT_FUNCTOR_PTR &pFunPtr = *it;
                PROPERTY_EVENT_FUNCTOR *pFun = pFunPtr.get();
                pFun->operator()(self, propertyName, oldVar, newVar, reason);
            }

            const std::string &className = xObject->GetPropertyString(excel::IObject::ClassName());
            std::map<std::string, std::list<PROPERTY_EVENT_FUNCTOR_PTR>>::iterator itClass = mtClassPropertyCallBackList.find(className);
            if (itClass != mtClassPropertyCallBackList.end()) {
                std::list<PROPERTY_EVENT_FUNCTOR_PTR>::iterator itList = itClass->second.begin();
                for (; itList != itClass->second.end(); itList++) {
                    PROPERTY_EVENT_FUNCTOR_PTR &pFunPtr = *itList;
                    PROPERTY_EVENT_FUNCTOR *pFun = pFunPtr.get();
                    pFun->operator()(self, propertyName, oldVar, newVar, reason);
                }
            }
        }
    }

    if (performance.CheckTimePoint(1)) {
        std::ostringstream os;
        os << "--------------- performance problem------------------- ";
        os << performance.TimeScope();
        os << "---------- ";
        os << propertyName;
        // m_log_->LogWarning(self, os, __FUNCTION__, __LINE__);
    }

    return 0;
}

std::shared_ptr<IObject> KernelModule::GetObject(const Guid &ident) { return GetElement(ident); }

int KernelModule::GetObjectByProperty(const int sceneID, const int groupID, const std::string &propertyName, const DataList &valueArg, DataList &list) {
    DataList varObjectList;
    GetGroupObjectList(sceneID, groupID, varObjectList);

    int nWorldCount = varObjectList.GetCount();
    for (int i = 0; i < nWorldCount; i++) {
        Guid ident = varObjectList.Object(i);
        if (this->FindProperty(ident, propertyName)) {
            DATA_TYPE eType = valueArg.Type(0);
            switch (eType) {
            case TDATA_INT: {
                int64_t nValue = GetPropertyInt(ident, propertyName.c_str());
                if (valueArg.Int(0) == nValue) {
                    list.Add(ident);
                }
            } break;
            case TDATA_STRING: {
                std::string strValue = GetPropertyString(ident, propertyName.c_str());
                std::string strCompareValue = valueArg.String(0);
                if (strValue == strCompareValue) {
                    list.Add(ident);
                }
            } break;
            case TDATA_OBJECT: {
                Guid identObject = GetPropertyObject(ident, propertyName.c_str());
                if (valueArg.Object(0) == identObject) {
                    list.Add(ident);
                }
            } break;
            default:
                break;
            }
        }
    }

    return list.GetCount();
}

bool KernelModule::ExistScene(const int sceneID) {
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->GetElement(sceneID);
    if (pSceneInfo) {
        return true;
    }

    return false;
}

bool KernelModule::ExistObject(const Guid &ident) { return ExistElement(ident); }

bool KernelModule::ObjectReady(const Guid &ident) {
    auto gameObject = GetElement(ident);
    if (gameObject) {
        return gameObject->ObjectReady();
    }

    return false;
}

bool KernelModule::ExistObject(const Guid &ident, const int sceneID, const int groupID) {
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->GetElement(sceneID);
    if (!pSceneInfo) {
        return false;
    }

    return pSceneInfo->ExistObjectInGroup(groupID, ident);
}

bool KernelModule::DestroySelf(const Guid &self) {
    mtDeleteSelfList.push_back(self);
    return true;
}

int KernelModule::OnRecordCommonEvent(const Guid &self, const RECORD_EVENT_DATA &eventData, const SquickData &oldVar, const SquickData &newVar) {
    Performance performance;

    std::shared_ptr<IObject> xObject = GetElement(self);
    if (xObject) {
        if (xObject->GetState() >= CLASS_OBJECT_EVENT::COE_CREATE_HASDATA) {
            std::list<RECORD_EVENT_FUNCTOR_PTR>::iterator it = mtCommonRecordCallBackList.begin();
            for (; it != mtCommonRecordCallBackList.end(); it++) {
                RECORD_EVENT_FUNCTOR_PTR &pFunPtr = *it;
                RECORD_EVENT_FUNCTOR *pFun = pFunPtr.get();
                pFun->operator()(self, eventData, oldVar, newVar);
            }
        }

        const std::string &className = xObject->GetPropertyString(excel::IObject::ClassName());
        std::map<std::string, std::list<RECORD_EVENT_FUNCTOR_PTR>>::iterator itClass = mtClassRecordCallBackList.find(className);
        if (itClass != mtClassRecordCallBackList.end()) {
            std::list<RECORD_EVENT_FUNCTOR_PTR>::iterator itList = itClass->second.begin();
            for (; itList != itClass->second.end(); itList++) {
                RECORD_EVENT_FUNCTOR_PTR &pFunPtr = *itList;
                RECORD_EVENT_FUNCTOR *pFun = pFunPtr.get();
                pFun->operator()(self, eventData, oldVar, newVar);
            }
        }
    }

    if (performance.CheckTimePoint(1)) {
        std::ostringstream os;
        os << "--------------- performance problem------------------- ";
        os << performance.TimeScope();
        os << "---------- ";
        os << eventData.recordName;
        os << " event type " << eventData.nOpType;
        // m_log_->LogWarning(self, os, __FUNCTION__, __LINE__);
    }

    return 0;
}

int KernelModule::OnClassCommonEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var) {
    Performance performance;

    std::list<CLASS_EVENT_FUNCTOR_PTR>::iterator it = mtCommonClassCallBackList.begin();
    for (; it != mtCommonClassCallBackList.end(); it++) {
        CLASS_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        CLASS_EVENT_FUNCTOR *pFun = pFunPtr.get();
        pFun->operator()(self, className, classEvent, var);
    }

    if (performance.CheckTimePoint(1)) {
        std::ostringstream os;
        os << "--------------- performance problem------------------- ";
        os << performance.TimeScope();
        os << "---------- ";
        os << className;
        os << " event type " << classEvent;
        // m_log_->LogWarning(self, os, __FUNCTION__, __LINE__);
    }

    return 0;
}

bool KernelModule::RegisterCommonClassEvent(const CLASS_EVENT_FUNCTOR_PTR &cb) {
    mtCommonClassCallBackList.push_back(cb);
    return true;
}

bool KernelModule::RegisterCommonPropertyEvent(const PROPERTY_EVENT_FUNCTOR_PTR &cb) {
    mtCommonPropertyCallBackList.push_back(cb);
    return true;
}

bool KernelModule::RegisterCommonRecordEvent(const RECORD_EVENT_FUNCTOR_PTR &cb) {
    mtCommonRecordCallBackList.push_back(cb);
    return true;
}

bool KernelModule::RegisterClassPropertyEvent(const std::string &className, const PROPERTY_EVENT_FUNCTOR_PTR &cb) {
    if (mtClassPropertyCallBackList.find(className) == mtClassPropertyCallBackList.end()) {
        std::list<PROPERTY_EVENT_FUNCTOR_PTR> xList;
        xList.push_back(cb);

        mtClassPropertyCallBackList.insert(std::map<std::string, std::list<PROPERTY_EVENT_FUNCTOR_PTR>>::value_type(className, xList));

        return true;
    }

    std::map<std::string, std::list<PROPERTY_EVENT_FUNCTOR_PTR>>::iterator it = mtClassPropertyCallBackList.find(className);
    it->second.push_back(cb);

    return false;
}

bool KernelModule::RegisterClassRecordEvent(const std::string &className, const RECORD_EVENT_FUNCTOR_PTR &cb) {
    if (mtClassRecordCallBackList.find(className) == mtClassRecordCallBackList.end()) {
        std::list<RECORD_EVENT_FUNCTOR_PTR> xList;
        xList.push_back(cb);

        mtClassRecordCallBackList.insert(std::map<std::string, std::list<RECORD_EVENT_FUNCTOR_PTR>>::value_type(className, xList));

        return true;
    }

    std::map<std::string, std::list<RECORD_EVENT_FUNCTOR_PTR>>::iterator it = mtClassRecordCallBackList.find(className);
    it->second.push_back(cb);

    return true;
}

bool KernelModule::LogSelfInfo(const Guid ident) { return false; }

bool KernelModule::AfterStart() {
    std::shared_ptr<IClass> pClass = m_class_->First();
    while (pClass) {
        IKernelModule::AddClassCallBack(pClass->GetClassName(), this, &KernelModule::OnClassCommonEvent);

        pClass = m_class_->Next();
    }

    return true;
}

bool KernelModule::DestroyAll() {
    std::shared_ptr<IObject> pObject = First();
    while (pObject) {
        mtDeleteSelfList.push_back(pObject->Self());

        pObject = Next();
    }

    Update();

    m_scene_->ClearAll();

    return true;
}

bool KernelModule::BeforeDestory() {
    DestroyAll();

    mvRandom.clear();
    mtCommonClassCallBackList.clear();
    mtCommonPropertyCallBackList.clear();
    mtCommonRecordCallBackList.clear();

    mtClassPropertyCallBackList.clear();
    mtClassRecordCallBackList.clear();

    return true;
}

int KernelModule::Random(int nStart, int nEnd) {
    if (++mxRandomItor == mvRandom.cend()) {
        mxRandomItor = mvRandom.cbegin();
    }

    return static_cast<int>((nEnd - nStart) * *mxRandomItor) + nStart;
}

float KernelModule::Random() {
    if (++mxRandomItor == mvRandom.cend()) {
        mxRandomItor = mvRandom.cbegin();
    }

    return *mxRandomItor;
}

bool KernelModule::AddClassCallBack(const std::string &className, const CLASS_EVENT_FUNCTOR_PTR &cb) { return m_class_->AddClassCallBack(className, cb); }

void KernelModule::ProcessMemFree() {
    if (nLastTime + 30 > pm_->GetNowTime()) {
        return;
    }

    nLastTime = pm_->GetNowTime();

    std::string info;

    // SQUICK_WILL_DO
    // MemoryCounter::PrintMemoryInfo(info);

    // m_log_->LogInfo(info, __FUNCTION__, __LINE__);

    // MemManager::GetSingletonPtr()->FreeMem();
    // MallocExtension::instance()->ReleaseFreeMemory();
}

bool KernelModule::DoEvent(const Guid &self, const std::string &className, CLASS_OBJECT_EVENT eEvent, const DataList &valueList) {
    return m_class_->DoEvent(self, className, eEvent, valueList);
}