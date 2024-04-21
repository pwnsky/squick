
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

WorldModule::WorldModule(IPluginManager *p) {
    is_update_ = true;
    nGUIDIndex = 0;
    nLastTime = 0;

    pm_ = p;

    nLastTime = pm_->GetNowTime();
    StartRandom();
}

WorldModule::~WorldModule() { ClearAll(); }

void WorldModule::StartRandom() {
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

bool WorldModule::Start() {
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

bool WorldModule::Destroy() { return true; }

bool WorldModule::Update() {
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

std::shared_ptr<IObject> WorldModule::CreateObject(const Guid &self, const int sceneID, const int groupID, const std::string &className,
                                                   const std::string &configIndex, const DataList &arg) {
    std::shared_ptr<IObject> pObject;
    Guid ident = self;

    std::shared_ptr<SceneInfo> pContainerInfo = m_scene_->GetElement(sceneID);
    if (!pContainerInfo) {
        LOG_ERROR("There is no scene<%v>", sceneID);
        return pObject;
    }

    if (!pContainerInfo->GetElement(groupID)) {
        LOG_ERROR("There is no group<%v>", groupID);
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
        LOG_ERROR("The object<%v> has Exists", ident.ToString());
        return pObject;
    }

    pObject = std::shared_ptr<IObject>(new Object(ident, pm_));
    AddElement(ident, pObject);

    if (pm_->UsingBackThread()) {
        m_thread_pool_->DoAsyncTask(
            Guid(), "",
            [=, this](ThreadTask &task) -> void {
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
                            pObject->AddPropertyCallBack(pStaticConfigPropertyInfo->GetKey(), this, &WorldModule::OnPropertyCommonEvent);

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
                            pObject->AddRecordCallBack(pConfigRecordInfo->GetName(), this, &WorldModule::OnRecordCommonEvent);

                            pConfigRecordInfo = pStaticClassRecordManager->Next();
                        }
                    }
                }
            },
            [=, this](ThreadTask &task) -> void {
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
                    [=, this](ThreadTask &task) -> void {
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
                    [=, this](ThreadTask &task) -> void {
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

                            LOG_INFO("%v", stream.str());

                            pObject->SetState(COE_CREATE_BEFORE_ATTACHDATA);
                            DoEvent(ident, className, pObject->GetState(), arg);
                        }

                        m_thread_pool_->DoAsyncTask(
                            Guid(), "",
                            [=, this](ThreadTask &task) -> void {
                                // back up thread
                                {
                                    pObject->SetState(COE_CREATE_LOADDATA);
                                    DoEvent(ident, className, pObject->GetState(), arg);
                                }
                            },
                            [=, this](ThreadTask &task) -> void {
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

                    pObject->AddPropertyCallBack(pStaticConfigPropertyInfo->GetKey(), this, &WorldModule::OnPropertyCommonEvent);

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

                    pObject->AddRecordCallBack(pConfigRecordInfo->GetName(), this, &WorldModule::OnRecordCommonEvent);

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

bool WorldModule::DestroyObject(const Guid &self) {
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

    LOG_ERROR("Guid<%v> There is no scene<%v>", self.ToString(), sceneID);

    return false;
}

bool WorldModule::FindProperty(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->FindProperty(propertyName);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return false;
}

bool WorldModule::SetPropertyInt(const Guid &self, const std::string &propertyName, const INT64 nValue, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyInt(propertyName, nValue, reason);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return false;
}

bool WorldModule::SetPropertyFloat(const Guid &self, const std::string &propertyName, const double dValue, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyFloat(propertyName, dValue, reason);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return false;
}

bool WorldModule::SetPropertyString(const Guid &self, const std::string &propertyName, const std::string &value, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyString(propertyName, value, reason);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return false;
}

bool WorldModule::SetPropertyObject(const Guid &self, const std::string &propertyName, const Guid &objectValue, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyObject(propertyName, objectValue, reason);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return false;
}

bool WorldModule::SetPropertyVector2(const Guid &self, const std::string &propertyName, const Vector2 &value, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyVector2(propertyName, value, reason);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return false;
}

bool WorldModule::SetPropertyVector3(const Guid &self, const std::string &propertyName, const Vector3 &value, const INT64 reason) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->SetPropertyVector3(propertyName, value, reason);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return false;
}

INT64 WorldModule::GetPropertyInt(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyInt(propertyName);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return NULL_INT;
}

int WorldModule::GetPropertyInt32(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyInt32(propertyName);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return (int)NULL_INT;
}

double WorldModule::GetPropertyFloat(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyFloat(propertyName);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return NULL_FLOAT;
}

const std::string &WorldModule::GetPropertyString(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyString(propertyName);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return "";
}

const Guid &WorldModule::GetPropertyObject(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyObject(propertyName);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return NULL_OBJECT;
}

const Vector2 &WorldModule::GetPropertyVector2(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyVector2(propertyName);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return NULL_VECTOR2;
}

const Vector3 &WorldModule::GetPropertyVector3(const Guid &self, const std::string &propertyName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetPropertyVector3(propertyName);
    }

    LOG_ERROR("Guid<%v> There is no property<%v>", self.ToString(), propertyName);
    return NULL_VECTOR3;
}

std::shared_ptr<IRecord> WorldModule::FindRecord(const Guid &self, const std::string &recordName) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordManager()->GetElement(recordName);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return nullptr;
}

bool WorldModule::ClearRecord(const Guid &self, const std::string &recordName) {
    std::shared_ptr<IRecord> pRecord = FindRecord(self, recordName);
    if (pRecord) {
        return pRecord->Clear();
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return false;
}

bool WorldModule::SetRecordInt(const Guid &self, const std::string &recordName, const int row, const int col, const INT64 nValue) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordInt(recordName, row, col, nValue)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordInt(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const INT64 value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordInt(recordName, row, colTag, value)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordFloat(const Guid &self, const std::string &recordName, const int row, const int col, const double dwValue) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordFloat(recordName, row, col, dwValue)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordFloat(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const double value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordFloat(recordName, row, colTag, value)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordString(const Guid &self, const std::string &recordName, const int row, const int col, const std::string &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordString(recordName, row, col, value)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordString(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const std::string &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordString(recordName, row, colTag, value)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordObject(const Guid &self, const std::string &recordName, const int row, const int col, const Guid &objectValue) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordObject(recordName, row, col, objectValue)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordObject(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Guid &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordObject(recordName, row, colTag, value)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordVector2(const Guid &self, const std::string &recordName, const int row, const int col, const Vector2 &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordVector2(recordName, row, col, value)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordVector2(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Vector2 &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordVector2(recordName, row, colTag, value)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordVector3(const Guid &self, const std::string &recordName, const int row, const int col, const Vector3 &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordVector3(recordName, row, col, value)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

bool WorldModule::SetRecordVector3(const Guid &self, const std::string &recordName, const int row, const std::string &colTag, const Vector3 &value) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        if (!pObject->SetRecordVector3(recordName, row, colTag, value)) {
            LOG_ERROR("Guid<%v> error for row or col, record_name<%v>", self.ToString(), recordName);
        } else {
            return true;
        }
    } else {
        LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    }

    return false;
}

INT64 WorldModule::GetRecordInt(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordInt(recordName, row, col);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return 0;
}

INT64 WorldModule::GetRecordInt(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordInt(recordName, row, colTag);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return 0;
}

double WorldModule::GetRecordFloat(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordFloat(recordName, row, col);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return 0.0;
}

double WorldModule::GetRecordFloat(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordFloat(recordName, row, colTag);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return 0.0;
}

const std::string &WorldModule::GetRecordString(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordString(recordName, row, col);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return "";
}

const std::string &WorldModule::GetRecordString(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordString(recordName, row, colTag);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return "";
}

const Guid &WorldModule::GetRecordObject(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordObject(recordName, row, col);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return NULL_OBJECT;
}

const Guid &WorldModule::GetRecordObject(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordObject(recordName, row, colTag);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return NULL_OBJECT;
}

const Vector2 &WorldModule::GetRecordVector2(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordVector2(recordName, row, col);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return NULL_VECTOR2;
}

const Vector2 &WorldModule::GetRecordVector2(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordVector2(recordName, row, colTag);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return NULL_VECTOR2;
}

const Vector3 &WorldModule::GetRecordVector3(const Guid &self, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordVector3(recordName, row, col);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return NULL_VECTOR3;
}

const Vector3 &WorldModule::GetRecordVector3(const Guid &self, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IObject> pObject = GetElement(self);
    if (pObject) {
        return pObject->GetRecordVector3(recordName, row, colTag);
    }

    LOG_ERROR("Guid<%v> There is no record<%v>", self.ToString(), recordName);
    return NULL_VECTOR3;
}

Guid WorldModule::CreateGUID() {
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

Guid WorldModule::CreatePlayerGUID() {
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

bool WorldModule::CreateScene(const int sceneID) {
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

bool WorldModule::DestroyScene(const int sceneID) {
    m_scene_->RemoveElement(sceneID);

    return true;
}

int WorldModule::GetOnLineCount() {
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

int WorldModule::GetMaxOnLineCount() {
    // test count 5000
    // and it should be define in a xml file

    return 10000;
}

int WorldModule::RequestGroupScene(const int sceneID) { return m_scene_->RequestGroupScene(sceneID); }

bool WorldModule::ReleaseGroupScene(const int sceneID, const int groupID) { return m_scene_->ReleaseGroupScene(sceneID, groupID); }

bool WorldModule::ExitGroupScene(const int sceneID, const int groupID) {
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->GetElement(sceneID);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(groupID);
        if (pGroupInfo) {
            return true;
        }
    }

    return false;
}

bool WorldModule::GetGroupObjectList(const int sceneID, const int groupID, DataList &list, const Guid &noSelf) {
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

int WorldModule::GetGroupObjectList(const int sceneID, const int groupID, const bool bPlayer, const Guid &noSelf) {
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

bool WorldModule::GetGroupObjectList(const int sceneID, const int groupID, DataList &list) { return GetGroupObjectList(sceneID, groupID, list, Guid()); }

bool WorldModule::GetGroupObjectList(const int sceneID, const int groupID, DataList &list, const bool bPlayer, const Guid &noSelf) {
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

bool WorldModule::GetGroupObjectList(const int sceneID, const int groupID, DataList &list, const bool bPlayer) {
    return GetGroupObjectList(sceneID, groupID, list, bPlayer, Guid());
}

bool WorldModule::GetGroupObjectList(const int sceneID, const int groupID, const std::string &className, DataList &list, const Guid &noSelf) {
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

bool WorldModule::GetGroupObjectList(const int sceneID, const int groupID, const std::string &className, DataList &list) {
    return GetGroupObjectList(sceneID, groupID, className, list, Guid());
}

bool WorldModule::LogStack() {
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

bool WorldModule::LogInfo(const Guid ident) {

    std::shared_ptr<IObject> pObject = GetObject(ident);
    if (pObject) {
        int sceneID = GetPropertyInt32(ident, excel::IObject::SceneID());
        int groupID = GetPropertyInt32(ident, excel::IObject::GroupID());
        LOG_INFO("Guid<%v> SceneID<%v>", ident.ToString(), sceneID);
    } else {
        LOG_ERROR("Guid<%v> There is no object", ident.ToString());
    }

    return true;
}

int WorldModule::OnPropertyCommonEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar,
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

std::shared_ptr<IObject> WorldModule::GetObject(const Guid &ident) { return GetElement(ident); }

int WorldModule::GetObjectByProperty(const int sceneID, const int groupID, const std::string &propertyName, const DataList &valueArg, DataList &list) {
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

bool WorldModule::ExistScene(const int sceneID) {
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->GetElement(sceneID);
    if (pSceneInfo) {
        return true;
    }

    return false;
}

bool WorldModule::ExistObject(const Guid &ident) { return ExistElement(ident); }

bool WorldModule::ObjectReady(const Guid &ident) {
    auto gameObject = GetElement(ident);
    if (gameObject) {
        return gameObject->ObjectReady();
    }

    return false;
}

bool WorldModule::ExistObject(const Guid &ident, const int sceneID, const int groupID) {
    std::shared_ptr<SceneInfo> pSceneInfo = m_scene_->GetElement(sceneID);
    if (!pSceneInfo) {
        return false;
    }

    return pSceneInfo->ExistObjectInGroup(groupID, ident);
}

bool WorldModule::DestroySelf(const Guid &self) {
    mtDeleteSelfList.push_back(self);
    return true;
}

int WorldModule::OnRecordCommonEvent(const Guid &self, const RECORD_EVENT_DATA &eventData, const SquickData &oldVar, const SquickData &newVar) {
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

int WorldModule::OnClassCommonEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var) {
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

bool WorldModule::RegisterCommonClassEvent(const CLASS_EVENT_FUNCTOR_PTR &cb) {
    mtCommonClassCallBackList.push_back(cb);
    return true;
}

bool WorldModule::RegisterCommonPropertyEvent(const PROPERTY_EVENT_FUNCTOR_PTR &cb) {
    mtCommonPropertyCallBackList.push_back(cb);
    return true;
}

bool WorldModule::RegisterCommonRecordEvent(const RECORD_EVENT_FUNCTOR_PTR &cb) {
    mtCommonRecordCallBackList.push_back(cb);
    return true;
}

bool WorldModule::RegisterClassPropertyEvent(const std::string &className, const PROPERTY_EVENT_FUNCTOR_PTR &cb) {
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

bool WorldModule::RegisterClassRecordEvent(const std::string &className, const RECORD_EVENT_FUNCTOR_PTR &cb) {
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

bool WorldModule::LogSelfInfo(const Guid ident) { return false; }

bool WorldModule::AfterStart() {
    std::shared_ptr<IClass> pClass = m_class_->First();
    while (pClass) {
        IWorldModule::AddClassCallBack(pClass->GetClassName(), this, &WorldModule::OnClassCommonEvent);

        pClass = m_class_->Next();
    }

    return true;
}

bool WorldModule::DestroyAll() {
    std::shared_ptr<IObject> pObject = First();
    while (pObject) {
        mtDeleteSelfList.push_back(pObject->Self());

        pObject = Next();
    }

    Update();

    m_scene_->ClearAll();

    return true;
}

bool WorldModule::BeforeDestroy() {
    DestroyAll();

    mvRandom.clear();
    mtCommonClassCallBackList.clear();
    mtCommonPropertyCallBackList.clear();
    mtCommonRecordCallBackList.clear();

    mtClassPropertyCallBackList.clear();
    mtClassRecordCallBackList.clear();

    return true;
}

int WorldModule::Random(int nStart, int nEnd) {
    if (++mxRandomItor == mvRandom.cend()) {
        mxRandomItor = mvRandom.cbegin();
    }

    return static_cast<int>((nEnd - nStart) * *mxRandomItor) + nStart;
}

float WorldModule::Random() {
    if (++mxRandomItor == mvRandom.cend()) {
        mxRandomItor = mvRandom.cbegin();
    }

    return *mxRandomItor;
}

bool WorldModule::AddClassCallBack(const std::string &className, const CLASS_EVENT_FUNCTOR_PTR &cb) { return m_class_->AddClassCallBack(className, cb); }

void WorldModule::ProcessMemFree() {
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

bool WorldModule::DoEvent(const Guid &self, const std::string &className, CLASS_OBJECT_EVENT eEvent, const DataList &valueList) {
    return m_class_->DoEvent(self, className, eEvent, valueList);
}