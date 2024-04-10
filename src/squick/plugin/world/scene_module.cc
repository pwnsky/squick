

#include "scene_module.h"
#include <squick/core/property_manager.h>
#include <squick/core/record_manager.h>
#include <struct/excel.h>

bool SceneModule::Start() {
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_event_ = pm_->FindModule<IEventModule>();
    m_pCellModule = pm_->FindModule<ICellModule>();

    m_kernel_->RegisterCommonClassEvent(this, &SceneModule::OnClassCommonEvent);
    m_kernel_->RegisterCommonPropertyEvent(this, &SceneModule::OnPropertyCommonEvent);
    m_kernel_->RegisterCommonRecordEvent(this, &SceneModule::OnRecordCommonEvent);

    return true;
}

bool SceneModule::AfterStart() {
    // init all scene, scene module cant create scene at Start() function as scene module depends IClassModule
    // and class module will load data at Start() function.
    // as a result, developer cant create game object at function AfterStart().
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Scene::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();

        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            int sceneID = lexical_cast<int>(strId);
            m_kernel_->CreateScene(sceneID);
        }
    }
    return true;
}

bool SceneModule::BeforeDestroy() {
    mvObjectEnterCallback.clear();
    mvObjectLeaveCallback.clear();
    mvPropertyEnterCallback.clear();
    mvRecordEnterCallback.clear();
    mvPropertySingleCallback.clear();
    mvRecordSingleCallback.clear();

    mvAfterEnterSceneCallback.clear();
    mvAfterEnterAndReadySceneCallback.clear();
    mvBeforeLeaveSceneCallback.clear();

    return true;
}

bool SceneModule::Destroy() { return true; }

bool SceneModule::Update() { return true; }

int SceneModule::RequestGroupScene(const int sceneID) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        int nNewGroupID = pSceneInfo->NewGroupID();
        if (!pSceneInfo->GetElement(nNewGroupID)) {
            std::shared_ptr<IPropertyManager> pPropertyManager(new PropertyManager(Guid(sceneID, nNewGroupID)));
            std::shared_ptr<IRecordManager> pRecordManager(new RecordManager(Guid(sceneID, nNewGroupID)));
            std::shared_ptr<SceneGroupInfo> pGroupInfo(new SceneGroupInfo(sceneID, nNewGroupID, pPropertyManager, pRecordManager));
            if (pGroupInfo) {
                Guid ident(sceneID, nNewGroupID);

                std::shared_ptr<IPropertyManager> pStaticClassPropertyManager = m_class_->GetClassPropertyManager(excel::Group::ThisName());
                std::shared_ptr<IRecordManager> pStaticClassRecordManager = m_class_->GetClassRecordManager(excel::Group::ThisName());
                if (pStaticClassPropertyManager && pStaticClassRecordManager) {
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

                        PROPERTY_EVENT_FUNCTOR functor = std::bind(&SceneModule::OnScenePropertyCommonEvent, this, std::placeholders::_1, std::placeholders::_2,
                                                                   std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
                        PROPERTY_EVENT_FUNCTOR_PTR functorPtr(new PROPERTY_EVENT_FUNCTOR(functor));
                        pPropertyManager->RegisterCallback(pStaticConfigPropertyInfo->GetKey(), functorPtr);

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

                        RECORD_EVENT_FUNCTOR functor = std::bind(&SceneModule::OnSceneRecordCommonEvent, this, std::placeholders::_1, std::placeholders::_2,
                                                                 std::placeholders::_3, std::placeholders::_4);
                        RECORD_EVENT_FUNCTOR_PTR functorPtr(new RECORD_EVENT_FUNCTOR(functor));

                        xRecord->AddRecordHook(functorPtr);
                        pConfigRecordInfo = pStaticClassRecordManager->Next();
                    }
                }

                pSceneInfo->AddElement(nNewGroupID, pGroupInfo);

                int sceneType = m_element_->GetPropertyInt32(std::to_string(sceneID), excel::Scene::Type());
                SceneGroupCreatedEvent(Guid(), sceneID, nNewGroupID, sceneType, DataList::Empty());

                m_pCellModule->CreateGroupCell(sceneID, nNewGroupID);
                return nNewGroupID;
            }
        }
    }

    return 0;
}

// 请求进入场景
bool SceneModule::RequestEnterScene(const Guid &self, const int sceneID, const int groupID, const int type, const Vector3 &pos, const DataList &argList) {
    if (groupID < 0) {
        return false;
    }

    const int nNowSceneID = m_kernel_->GetPropertyInt32(self, excel::Player::SceneID());
    const int nNowGroupID = m_kernel_->GetPropertyInt32(self, excel::Player::GroupID());

    if (nNowSceneID == sceneID && nNowGroupID == groupID) {
        m_log_->LogInfo(self, "in same scene and group but it not a clone scene " + std::to_string(sceneID));

        return false;
    }

    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (!pSceneInfo) {
        return false;
    }

    /*
    if (!pSceneInfo->ExistElement(nNewGroupID))
    {
            return false;
    }
    */

    int nEnterConditionCode = EnterSceneCondition(self, sceneID, groupID, type, argList);
    if (nEnterConditionCode != 0) {
        m_log_->LogInfo(self, "before enter condition code: " + std::to_string(nEnterConditionCode), __FUNCTION__, __LINE__);
        return false;
    }

    if (!SwitchScene(self, sceneID, groupID, type, pos, 0.0f, argList)) {
        m_log_->LogInfo(self, "SwitchScene failed " + std::to_string(sceneID));

        return false;
    }

    return true;
}

bool SceneModule::ReleaseGroupScene(const int sceneID, const int groupID) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        if (groupID > 0) {
            DestroySceneNPC(sceneID, groupID);

            m_pCellModule->DestroyGroupCell(sceneID, groupID);

            int sceneType = m_element_->GetPropertyInt32(std::to_string(sceneID), excel::Scene::Type());
            SceneGroupDestroyedEvent(Guid(), sceneID, groupID, sceneType, DataList::Empty());

            pSceneInfo->RemoveElement(groupID);

            return true;
        }
    }

    return false;
}

bool SceneModule::LeaveSceneGroup(const Guid &self) {
    std::shared_ptr<IObject> pObject = m_kernel_->GetObject(self);
    if (pObject) {
        int nOldSceneID = pObject->GetPropertyInt32(excel::Scene::SceneID());
        int nOldGroupID = pObject->GetPropertyInt32(excel::Scene::GroupID());
        if (nOldGroupID <= 0) {
            // m_log_->LogError(self, "no this group == 0 " + std::to_string(nOldSceneID), __FUNCTION__, __LINE__);
            // return false;
        }

        std::shared_ptr<SceneInfo> pOldSceneInfo = this->GetElement(nOldSceneID);
        if (!pOldSceneInfo) {
            m_log_->LogError(self, "no this container " + std::to_string(nOldSceneID), __FUNCTION__, __LINE__);
            return false;
        }

        if (!pOldSceneInfo->GetElement(nOldGroupID)) {
            m_log_->LogError(self, "no this group " + std::to_string(nOldGroupID), __FUNCTION__, __LINE__);
            return false;
        }
        /////////

        const Vector3 &lastPos = m_kernel_->GetPropertyVector3(self, excel::IObject::Position());
        BeforeLeaveSceneGroup(self, nOldSceneID, nOldGroupID, 0, DataList::Empty());

        const Guid lastCell = m_pCellModule->ComputeCellID(lastPos);
        OnMoveCellEvent(self, nOldSceneID, nOldGroupID, lastCell, Guid());

        pOldSceneInfo->RemoveObjectFromGroup(nOldGroupID, self, true);

        // if (nTargetSceneID != nOldSceneID)
        {
            pObject->SetPropertyInt(excel::Scene::GroupID(), 0);
            /////////
            AfterLeaveSceneGroup(self, nOldSceneID, nOldGroupID, 0, DataList::Empty());
        }

        return true;
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

    return false;
}

const std::vector<int> &SceneModule::GetGroups(const int sceneID) {
    static std::vector<int> vec;
    vec.clear();

    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->First();
        while (pGroupInfo) {
            vec.push_back(pGroupInfo->groupID);

            pGroupInfo = pSceneInfo->Next();
        }
    }

    return vec;
}

bool SceneModule::AddSeedData(const int sceneID, const std::string &seedID, const std::string &configID, const Vector3 &vPos, const int nWeight) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        return pSceneInfo->AddSeedObjectInfo(seedID, configID, vPos, nWeight);
    }

    return false;
}

const Vector3 &SceneModule::GetSeedPos(const int sceneID, const std::string &seedID) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        auto seedDnata = pSceneInfo->GetSeedObjectInfo(seedID);
        if (seedDnata) {
            return seedDnata->vSeedPos;
        }
    }

    return Vector3::Zero();
}

const int SceneModule::GetSeedPWeight(const int sceneID, const std::string &seedID) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        auto seedDnata = pSceneInfo->GetSeedObjectInfo(seedID);
        if (seedDnata) {
            return seedDnata->nWeight;
        }
    }

    return 0;
}

bool SceneModule::AddRelivePosition(const int sceneID, const int nIndex, const Vector3 &vPos) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        return pSceneInfo->AddReliveInfo(nIndex, vPos);
    }

    return false;
}

const Vector3 &SceneModule::GetRelivePosition(const int sceneID, const int nIndex) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        return pSceneInfo->GetReliveInfo(nIndex);
    }

    return Vector3::Zero();
}

bool SceneModule::AddTagPosition(const int sceneID, const int nIndex, const Vector3 &vPos) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        return pSceneInfo->AddTagInfo(nIndex, vPos);
    }

    return false;
}

const Vector3 &SceneModule::GetTagPosition(const int sceneID, const int nIndex) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        return pSceneInfo->GetTagInfo(nIndex);
    }

    return Vector3::Zero();
}

bool SceneModule::AddObjectEnterCallBack(const OBJECT_ENTER_EVENT_FUNCTOR_PTR &cb) {
    mvObjectEnterCallback.push_back(cb);
    return true;
}

bool SceneModule::AddObjectDataFinishedCallBack(const OBJECT_ENTER_EVENT_FUNCTOR_PTR &cb) {
    mvObjectDataFinishedCallBack.push_back(cb);
    return true;
}

bool SceneModule::AddObjectLeaveCallBack(const OBJECT_LEAVE_EVENT_FUNCTOR_PTR &cb) {
    mvObjectLeaveCallback.push_back(cb);
    return true;
}

bool SceneModule::AddPropertyEnterCallBack(const PROPERTY_ENTER_EVENT_FUNCTOR_PTR &cb) {
    mvPropertyEnterCallback.push_back(cb);
    return true;
}

bool SceneModule::AddRecordEnterCallBack(const RECORD_ENTER_EVENT_FUNCTOR_PTR &cb) {
    mvRecordEnterCallback.push_back(cb);
    return true;
}

bool SceneModule::AddPropertyEventCallBack(const PROPERTY_SINGLE_EVENT_FUNCTOR_PTR &cb) {
    mvPropertySingleCallback.push_back(cb);
    return true;
}

bool SceneModule::AddRecordEventCallBack(const RECORD_SINGLE_EVENT_FUNCTOR_PTR &cb) {
    mvRecordSingleCallback.push_back(cb);
    return true;
}

bool SceneModule::AddEnterSceneConditionCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) {
    mvEnterSceneConditionCallback.push_back(cb);
    return true;
}

bool SceneModule::AddBeforeEnterSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) {
    mvBeforeEnterSceneCallback.push_back(cb);
    return true;
}

bool SceneModule::AddAfterEnterSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) {
    mvAfterEnterSceneCallback.push_back(cb);
    return true;
}

bool SceneModule::AddSwapSceneEventCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) {
    mvOnSwapSceneCallback.push_back(cb);
    return true;
}

bool SceneModule::AddBeforeLeaveSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) {
    mvBeforeLeaveSceneCallback.push_back(cb);
    return true;
}

bool SceneModule::AddAfterLeaveSceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) {
    mvAfterLeaveSceneCallback.push_back(cb);
    return true;
}

bool SceneModule::AddSceneGroupCreatedCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) {

    mvSceneGroupCreatedCallback.push_back(cb);
    return true;
}

bool SceneModule::AddSceneGroupDestroyedCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) {
    mvSceneGroupDestroyedCallback.push_back(cb);
    return true;
}

bool SceneModule::CreateSceneNPC(const int sceneID, const int groupID) { return CreateSceneNPC(sceneID, groupID, DataList::Empty()); }

bool SceneModule::CreateSceneNPC(const int sceneID, const int groupID, const DataList &argList) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (!pSceneInfo) {
        return false;
    }

    std::shared_ptr<SceneSeedResource> pResource = pSceneInfo->mtSceneResourceConfig.First();
    for (; pResource; pResource = pSceneInfo->mtSceneResourceConfig.Next()) {
        int nWeight = m_kernel_->Random(0, 100);
        if (nWeight <= pResource->nWeight) {
            DataList arg;
            arg << excel::IObject::Position() << pResource->vSeedPos;
            // arg << SquickProtocol::NPC::SeedID() << pResource->seedID;
            arg.Append(argList);

            // m_kernel_->CreateObject(Guid(), sceneID, groupID, SquickProtocol::NPC::ThisName(), pResource->configID, arg);
        }
    }

    return false;
}

bool SceneModule::DestroySceneNPC(const int sceneID, const int groupID) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(sceneID);
    if (pSceneInfo) {
        if (pSceneInfo->GetElement(groupID)) {
            DataList xMonsterlistObject;
            if (m_kernel_->GetGroupObjectList(sceneID, groupID, xMonsterlistObject, false)) {
                for (int i = 0; i < xMonsterlistObject.GetCount(); ++i) {
                    Guid ident = xMonsterlistObject.Object(i);
                    m_kernel_->DestroyObject(ident);
                }
            }

            pSceneInfo->RemoveElement(groupID);

            return true;
        }
    }

    return false;
}

bool SceneModule::RemoveSwapSceneEventCallBack() {
    mvOnSwapSceneCallback.clear();

    return true;
}

bool SceneModule::SetPropertyInt(const int scene, const int group, const std::string &propertyName, const INT64 nValue) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->SetPropertyInt(propertyName, nValue);
        }
    }

    return false;
}

bool SceneModule::SetPropertyFloat(const int scene, const int group, const std::string &propertyName, const double dValue) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->SetPropertyFloat(propertyName, dValue);
        }
    }

    return false;
}

bool SceneModule::SetPropertyString(const int scene, const int group, const std::string &propertyName, const std::string &value) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->SetPropertyString(propertyName, value);
        }
    }

    return false;
}

bool SceneModule::SetPropertyObject(const int scene, const int group, const std::string &propertyName, const Guid &objectValue) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->SetPropertyObject(propertyName, objectValue);
        }
    }

    return false;
}

bool SceneModule::SetPropertyVector2(const int scene, const int group, const std::string &propertyName, const Vector2 &value) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->SetPropertyVector2(propertyName, value);
        }
    }

    return false;
}

bool SceneModule::SetPropertyVector3(const int scene, const int group, const std::string &propertyName, const Vector3 &value) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->SetPropertyVector3(propertyName, value);
        }
    }

    return false;
}

INT64 SceneModule::GetPropertyInt(const int scene, const int group, const std::string &propertyName) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->GetPropertyInt(propertyName);
        }
    }

    return 0;
}

int SceneModule::GetPropertyInt32(const int scene, const int group, const std::string &propertyName) { return (int)GetPropertyInt(scene, group, propertyName); }

double SceneModule::GetPropertyFloat(const int scene, const int group, const std::string &propertyName) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->GetPropertyFloat(propertyName);
        }
    }

    return 0.0;
}

const std::string &SceneModule::GetPropertyString(const int scene, const int group, const std::string &propertyName) {

    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->GetPropertyString(propertyName);
        }
    }

    static std::string emptyString("");
    return emptyString;
}

const Guid &SceneModule::GetPropertyObject(const int scene, const int group, const std::string &propertyName) {

    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->GetPropertyObject(propertyName);
        }
    }

    return NULL_OBJECT;
}

const Vector2 &SceneModule::GetPropertyVector2(const int scene, const int group, const std::string &propertyName) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->GetPropertyVector2(propertyName);
        }
    }

    return Vector2::Zero();
}

const Vector3 &SceneModule::GetPropertyVector3(const int scene, const int group, const std::string &propertyName) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager->GetPropertyVector3(propertyName);
        }
    }

    return Vector3::Zero();
}

std::shared_ptr<IPropertyManager> SceneModule::FindPropertyManager(const int scene, const int group) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxPropertyManager;
        }
    }

    return nullptr;
}

std::shared_ptr<IRecordManager> SceneModule::FindRecordManager(const int scene, const int group) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxRecordManager;
        }
    }

    return nullptr;
}

std::shared_ptr<IRecord> SceneModule::FindRecord(const int scene, const int group, const std::string &recordName) {
    std::shared_ptr<SceneInfo> pSceneInfo = GetElement(scene);
    if (pSceneInfo) {
        std::shared_ptr<SceneGroupInfo> pGroupInfo = pSceneInfo->GetElement(group);
        if (pGroupInfo) {
            return pGroupInfo->mxRecordManager->GetElement(recordName);
        }
    }

    return nullptr;
}

bool SceneModule::ClearRecord(const int scene, const int group, const std::string &recordName) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->Clear();
    }

    return false;
}

bool SceneModule::SetRecordInt(const int scene, const int group, const std::string &recordName, const int row, const int col, const INT64 nValue) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetInt(row, col, nValue);
    }

    return false;
}

bool SceneModule::SetRecordFloat(const int scene, const int group, const std::string &recordName, const int row, const int col, const double dwValue) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetFloat(row, col, dwValue);
    }

    return false;
}

bool SceneModule::SetRecordString(const int scene, const int group, const std::string &recordName, const int row, const int col, const std::string &value) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetString(row, col, value);
    }

    return false;
}

bool SceneModule::SetRecordObject(const int scene, const int group, const std::string &recordName, const int row, const int col, const Guid &objectValue) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetObject(row, col, objectValue);
    }

    return false;
}

bool SceneModule::SetRecordVector2(const int scene, const int group, const std::string &recordName, const int row, const int col, const Vector2 &value) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetVector2(row, col, value);
    }

    return false;
}

bool SceneModule::SetRecordVector3(const int scene, const int group, const std::string &recordName, const int row, const int col, const Vector3 &value) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetVector3(row, col, value);
    }

    return false;
}

bool SceneModule::SetRecordInt(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag, const INT64 value) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetInt(row, colTag, value);
    }

    return false;
}

bool SceneModule::SetRecordFloat(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag,
                                 const double value) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetFloat(row, colTag, value);
    }

    return false;
}

bool SceneModule::SetRecordString(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag,
                                  const std::string &value) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetString(row, colTag, value);
    }

    return false;
}

bool SceneModule::SetRecordObject(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag,
                                  const Guid &value) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetObject(row, colTag, value);
    }

    return false;
}

bool SceneModule::SetRecordVector2(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag,
                                   const Vector2 &value) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetVector2(row, colTag, value);
    }

    return false;
}

bool SceneModule::SetRecordVector3(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag,
                                   const Vector3 &value) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->SetVector3(row, colTag, value);
    }

    return false;
}

INT64 SceneModule::GetRecordInt(const int scene, const int group, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetInt(row, col);
    }

    return 0;
}

double SceneModule::GetRecordFloat(const int scene, const int group, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetFloat(row, col);
    }

    return 0.0;
}

const std::string &SceneModule::GetRecordString(const int scene, const int group, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetString(row, col);
    }

    return NULL_STR;
}

const Guid &SceneModule::GetRecordObject(const int scene, const int group, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetObject(row, col);
    }

    return NULL_OBJECT;
}

const Vector2 &SceneModule::GetRecordVector2(const int scene, const int group, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetVector2(row, col);
    }

    return Vector2::Zero();
}

const Vector3 &SceneModule::GetRecordVector3(const int scene, const int group, const std::string &recordName, const int row, const int col) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetVector3(row, col);
    }

    return Vector3::Zero();
}

INT64 SceneModule::GetRecordInt(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetInt(row, colTag);
    }

    return 0;
}

double SceneModule::GetRecordFloat(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetFloat(row, colTag);
    }

    return 0.0;
}

const std::string &SceneModule::GetRecordString(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetString(row, colTag);
    }

    return NULL_STR;
}

const Guid &SceneModule::GetRecordObject(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetObject(row, colTag);
    }

    return NULL_OBJECT;
}

const Vector2 &SceneModule::GetRecordVector2(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetVector2(row, colTag);
    }

    return Vector2::Zero();
}

const Vector3 &SceneModule::GetRecordVector3(const int scene, const int group, const std::string &recordName, const int row, const std::string &colTag) {
    std::shared_ptr<IRecord> xRecord = FindRecord(scene, group, recordName);
    if (xRecord) {
        return xRecord->GetVector3(row, colTag);
    }

    return Vector3::Zero();
}

bool SceneModule::AddGroupPropertyCallBack(const std::string &name, const PROPERTY_EVENT_FUNCTOR_PTR &cb) {
    if (mtGroupPropertyCallBackList.find(name) == mtGroupPropertyCallBackList.end()) {
        std::list<PROPERTY_EVENT_FUNCTOR_PTR> xList;
        xList.push_back(cb);

        mtGroupPropertyCallBackList.insert(std::map<std::string, std::list<PROPERTY_EVENT_FUNCTOR_PTR>>::value_type(name, xList));

        return true;
    }

    std::map<std::string, std::list<PROPERTY_EVENT_FUNCTOR_PTR>>::iterator it = mtGroupPropertyCallBackList.find(name);
    it->second.push_back(cb);

    return true;
}

bool SceneModule::AddGroupRecordCallBack(const std::string &name, const RECORD_EVENT_FUNCTOR_PTR &cb) {
    if (mtGroupRecordCallBackList.find(name) == mtGroupRecordCallBackList.end()) {
        std::list<RECORD_EVENT_FUNCTOR_PTR> xList;
        xList.push_back(cb);

        mtGroupRecordCallBackList.insert(std::map<std::string, std::list<RECORD_EVENT_FUNCTOR_PTR>>::value_type(name, xList));

        return true;
    }

    std::map<std::string, std::list<RECORD_EVENT_FUNCTOR_PTR>>::iterator it = mtGroupRecordCallBackList.find(name);
    it->second.push_back(cb);

    return true;
}

bool SceneModule::AddGroupPropertyCommCallBack(const PROPERTY_EVENT_FUNCTOR_PTR &cb) {
    mtGroupPropertyCommCallBackList.push_back(cb);

    return true;
}

bool SceneModule::AddGroupRecordCommCallBack(const RECORD_EVENT_FUNCTOR_PTR &cb) {
    mtGroupRecordCallCommBackList.push_back(cb);

    return true;
}

bool SceneModule::SwitchScene(const Guid &self, const int nTargetSceneID, const int nTargetGroupID, const int type, const Vector3 v, const float fOrient,
                              const DataList &arg) {
    std::shared_ptr<IObject> pObject = m_kernel_->GetObject(self);
    if (pObject) {
        int nOldSceneID = pObject->GetPropertyInt32(excel::Scene::SceneID());
        int nOldGroupID = pObject->GetPropertyInt32(excel::Scene::GroupID());

        std::shared_ptr<SceneInfo> pOldSceneInfo = this->GetElement(nOldSceneID);
        std::shared_ptr<SceneInfo> pNewSceneInfo = this->GetElement(nTargetSceneID);
        if (!pOldSceneInfo) {
            m_log_->LogError(self, "no this container " + std::to_string(nOldSceneID), __FUNCTION__, __LINE__);
            return false;
        }

        if (!pNewSceneInfo) {
            m_log_->LogError(self, "no this container " + std::to_string(nTargetSceneID), __FUNCTION__, __LINE__);
            return false;
        }

        if (!pNewSceneInfo->GetElement(nTargetGroupID)) {
            m_log_->LogError(self, "no this group " + std::to_string(nTargetGroupID), __FUNCTION__, __LINE__);
            return false;
        }
        /////////

        const Vector3 &lastPos = m_kernel_->GetPropertyVector3(self, excel::IObject::Position());
        BeforeLeaveSceneGroup(self, nOldSceneID, nOldGroupID, type, arg);

        const Guid lastCell = m_pCellModule->ComputeCellID(lastPos);
        OnMoveCellEvent(self, nOldSceneID, nOldGroupID, lastCell, Guid());

        pOldSceneInfo->RemoveObjectFromGroup(nOldGroupID, self, true);

        // if (nTargetSceneID != nOldSceneID)
        {
            pObject->SetPropertyInt(excel::Scene::GroupID(), 0);
            /////////
            AfterLeaveSceneGroup(self, nOldSceneID, nOldGroupID, type, arg);

            pObject->SetPropertyInt(excel::Scene::SceneID(), nTargetSceneID);

            OnSwapSceneEvent(self, nTargetSceneID, nTargetGroupID, type, arg);
        }

        ////////
        BeforeEnterSceneGroup(self, nTargetSceneID, nTargetGroupID, type, arg);

        pNewSceneInfo->AddObjectToGroup(nTargetGroupID, self, true);
        pObject->SetPropertyInt(excel::Scene::GroupID(), nTargetGroupID);

        const Guid newCell = m_pCellModule->ComputeCellID(v);
        OnMoveCellEvent(self, nTargetGroupID, nTargetGroupID, Guid(), newCell);

        pObject->SetPropertyVector3(excel::IObject::MoveTo(), v);

        /////////
        AfterEnterSceneGroup(self, nTargetSceneID, nTargetGroupID, type, arg);

        AfterEnterAndReadySceneGroup(self, nTargetSceneID, nTargetGroupID, type, arg);

        return true;
    }

    m_log_->LogObject(ILogModule::NLL_ERROR_NORMAL, self, "There is no object", __FUNCTION__, __LINE__);

    return false;
}

int SceneModule::OnScenePropertyCommonEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar,
                                            const INT64 reason) {
    auto itList = mtGroupPropertyCommCallBackList.begin();
    for (; itList != mtGroupPropertyCommCallBackList.end(); itList++) {
        PROPERTY_EVENT_FUNCTOR_PTR &pFunPtr = *itList;
        PROPERTY_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, propertyName, oldVar, newVar, reason);
    }

    auto it = mtGroupPropertyCallBackList.find(propertyName);
    if (it != mtGroupPropertyCallBackList.end()) {
        auto itList = it->second.begin();
        for (; itList != it->second.end(); itList++) {
            PROPERTY_EVENT_FUNCTOR_PTR &pFunPtr = *itList;
            PROPERTY_EVENT_FUNCTOR *pFunc = pFunPtr.get();
            pFunc->operator()(self, propertyName, oldVar, newVar, reason);
        }
    }

    return 0;
}

int SceneModule::OnSceneRecordCommonEvent(const Guid &self, const RECORD_EVENT_DATA &eventData, const SquickData &oldVar, const SquickData &newVar) {
    auto itList = mtGroupRecordCallCommBackList.begin();
    for (; itList != mtGroupRecordCallCommBackList.end(); itList++) {
        RECORD_EVENT_FUNCTOR_PTR &pFunPtr = *itList;
        RECORD_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, eventData, oldVar, newVar);
    }

    auto it = mtGroupRecordCallBackList.find(eventData.recordName);
    if (it != mtGroupRecordCallBackList.end()) {
        auto itList = it->second.begin();
        for (; itList != it->second.end(); itList++) {
            RECORD_EVENT_FUNCTOR_PTR &pFunPtr = *itList;
            RECORD_EVENT_FUNCTOR *pFunc = pFunPtr.get();
            pFunc->operator()(self, eventData, oldVar, newVar);
        }
    }

    return 0;
}

int SceneModule::OnPropertyCommonEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar,
                                       const INT64 reason) {
    const std::string &className = m_kernel_->GetPropertyString(self, excel::IObject::ClassName());
    if (className == excel::Player::ThisName()) {
        // only player can change grupid and sceneid
        if (excel::Player::GroupID() == propertyName) {
            OnPlayerGroupEvent(self, propertyName, oldVar, newVar);
            return 0;
        }

        if (excel::Player::SceneID() == propertyName) {
            OnPlayerSceneEvent(self, propertyName, oldVar, newVar);
            return 0;
        }
    }

    DataList valueBroadCaseList;
    if (GetBroadCastObject(self, propertyName, false, valueBroadCaseList) <= 0) {
        return 0;
    }

    OnPropertyEvent(self, propertyName, oldVar, newVar, valueBroadCaseList, reason);

    return 0;
}

int SceneModule::OnRecordCommonEvent(const Guid &self, const RECORD_EVENT_DATA &eventData, const SquickData &oldVar, const SquickData &newVar) {
    const std::string &recordName = eventData.recordName;

    int nObjectGroupID = m_kernel_->GetPropertyInt32(self, excel::Player::GroupID());

    if (nObjectGroupID < 0) {
        return 0;
    }

    DataList valueBroadCaseList;
    GetBroadCastObject(self, recordName, true, valueBroadCaseList);

    OnRecordEvent(self, recordName, eventData, oldVar, newVar, valueBroadCaseList);

    return 0;
}

int SceneModule::OnClassCommonEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var) {
    if (CLASS_OBJECT_EVENT::COE_DESTROY == classEvent) {
        const int nObjectSceneID = m_kernel_->GetPropertyInt32(self, excel::IObject::SceneID());
        const int nObjectGroupID = m_kernel_->GetPropertyInt32(self, excel::IObject::GroupID());

        if (nObjectGroupID < 0 || nObjectSceneID <= 0) {
            return 0;
        }

        const Vector3 &pos = m_kernel_->GetPropertyVector3(self, excel::Player::Position());
        DataList valueAllPlayrNoSelfList;
        m_pCellModule->GetCellObjectList(nObjectSceneID, nObjectGroupID, pos, valueAllPlayrNoSelfList, true, self);

        // tell other people that you want to leave from this scene or this group
        // every one want to know you want to leave notmater you are a monster maybe you are a player
        OnObjectListLeave(valueAllPlayrNoSelfList, DataList() << self);
    }

    else if (CLASS_OBJECT_EVENT::COE_CREATE_NODATA == classEvent) {

    } else if (CLASS_OBJECT_EVENT::COE_CREATE_LOADDATA == classEvent) {
    } else if (CLASS_OBJECT_EVENT::COE_CREATE_HASDATA == classEvent) {
        DataList selfVar;
        selfVar << self;

        if (className == excel::Player::ThisName()) {
            // tell youself<client>, u want to enter this scene or this group
            OnObjectListEnter(selfVar, selfVar);

            // tell youself<client>, u want to broad your properties and records to youself
            OnPropertyEnter(selfVar, self);
            OnRecordEnter(selfVar, self);

            OnObjectListEnterFinished(selfVar, selfVar);
        } else {
            const int nObjectSceneID = m_kernel_->GetPropertyInt32(self, excel::IObject::SceneID());
            const int nObjectGroupID = m_kernel_->GetPropertyInt32(self, excel::IObject::GroupID());

            if (nObjectGroupID < 0 || nObjectSceneID <= 0) {
                return 0;
            }
            const Vector3 &pos = m_kernel_->GetPropertyVector3(self, excel::Player::Position());
            DataList valueAllPlayrObjectList;
            m_pCellModule->GetCellObjectList(nObjectSceneID, nObjectGroupID, pos, valueAllPlayrObjectList, true);

            // monster or others need to tell all player
            OnObjectListEnter(valueAllPlayrObjectList, selfVar);
            OnPropertyEnter(valueAllPlayrObjectList, self);
            OnRecordEnter(valueAllPlayrObjectList, self);

            OnObjectListEnterFinished(valueAllPlayrObjectList, selfVar);
        }
    } else if (CLASS_OBJECT_EVENT::COE_CREATE_FINISH == classEvent) {
    }

    return 0;
}

int SceneModule::OnPlayerGroupEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar) {
    // this event only happened in the same scene
    const int sceneID = m_kernel_->GetPropertyInt32(self, excel::IObject::SceneID());
    const Vector3 position = m_kernel_->GetPropertyVector3(self, excel::IObject::Position());
    int nOldGroupID = oldVar.GetInt32();
    int nNewGroupID = newVar.GetInt32();

    // maybe form 0, maybe not, only three stuation
    // example1: 0 -> 1 ==> new_group > 0 && old_group <= 0
    // example2: 1 -> 2 ==> new_group > 0 && old_group > 0
    // example3: 5 -> 0 ==> new_group <= 0 && old_group > 0

    DataList selfVar;
    selfVar << self;

    if (nNewGroupID > 0) {
        if (nOldGroupID > 0) {
            // example2: 1 -> 2 ==> new_group > 0 && old_group > 0
            // step1: leave
            DataList valueAllOldNPCListNoSelf;
            DataList valueAllOldPlayerListNoSelf;
            m_pCellModule->GetCellObjectList(sceneID, nOldGroupID, position, valueAllOldNPCListNoSelf, false, self);
            m_pCellModule->GetCellObjectList(sceneID, nOldGroupID, position, valueAllOldPlayerListNoSelf, true, self);

            OnObjectListLeave(valueAllOldPlayerListNoSelf, selfVar);
            OnObjectListLeave(selfVar, valueAllOldPlayerListNoSelf);
            OnObjectListLeave(selfVar, valueAllOldNPCListNoSelf);
        } else {
            // example1: 0 -> 1 == > new_group > 0 && old_group <= 0
            // only use step2 that enough
        }

        // step2: enter
        DataList valueAllNewNPCListNoSelf;
        DataList valueAllNewPlayerListNoSelf;

        m_pCellModule->GetCellObjectList(sceneID, nNewGroupID, position, valueAllNewNPCListNoSelf, false, self);
        m_pCellModule->GetCellObjectList(sceneID, nNewGroupID, position, valueAllNewPlayerListNoSelf, true, self);

        OnObjectListEnter(valueAllNewPlayerListNoSelf, selfVar);
        OnObjectListEnter(selfVar, valueAllNewPlayerListNoSelf);
        OnObjectListEnter(selfVar, valueAllNewNPCListNoSelf);

        // bc others data to u
        DataList identOldVar;
        identOldVar.Add(Guid());

        for (int i = 0; i < valueAllNewNPCListNoSelf.GetCount(); i++) {
            Guid identOld = valueAllNewNPCListNoSelf.Object(i);
            identOldVar.SetObject(0, identOld);

            OnPropertyEnter(selfVar, identOld);
            OnRecordEnter(selfVar, identOld);

            OnObjectListEnterFinished(selfVar, identOldVar);
        }

        // bc others data to u
        for (int i = 0; i < valueAllNewPlayerListNoSelf.GetCount(); i++) {
            Guid identOld = valueAllNewPlayerListNoSelf.Object(i);
            identOldVar.SetObject(0, identOld);

            OnPropertyEnter(selfVar, identOld);
            OnRecordEnter(selfVar, identOld);

            OnObjectListEnterFinished(selfVar, identOldVar);
        }

        // bc u data to others
        OnPropertyEnter(valueAllNewPlayerListNoSelf, self);
        OnRecordEnter(valueAllNewPlayerListNoSelf, self);

        OnObjectListEnterFinished(valueAllNewPlayerListNoSelf, selfVar);
    } else {
        if (nOldGroupID > 0) {
            // example3: 5 -> 0 ==> new_group <= 0 && old_group > 0
            // step1: leave
            DataList valueAllOldNPCListNoSelf;
            DataList valueAllOldPlayerListNoSelf;
            m_pCellModule->GetCellObjectList(sceneID, nOldGroupID, position, valueAllOldNPCListNoSelf, false, self);
            m_pCellModule->GetCellObjectList(sceneID, nOldGroupID, position, valueAllOldPlayerListNoSelf, true, self);

            OnObjectListLeave(valueAllOldPlayerListNoSelf, selfVar);
            OnObjectListLeave(selfVar, valueAllOldPlayerListNoSelf);
            OnObjectListLeave(selfVar, valueAllOldNPCListNoSelf);
        }
    }

    return 0;
}

int SceneModule::OnPlayerSceneEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar) {
    // no more player in this group of this scene at the same time
    // so now only one player(that you) in this group of this scene
    // BTW, most of time, we dont create monsters in the group 0
    // so no one at this group but u

    // therefore u just need post the new scene id to you self
    OnPropertyEvent(self, propertyName, oldVar, newVar, DataList() << self, 0);
    return 0;
}

int SceneModule::GetBroadCastObject(const Guid &self, const std::string &propertyName, const bool bTable, DataList &valueObject) {
    const int nObjectContainerID = m_kernel_->GetPropertyInt32(self, excel::IObject::SceneID());
    const int nObjectGroupID = m_kernel_->GetPropertyInt32(self, excel::IObject::GroupID());
    const Vector3 &position = m_kernel_->GetPropertyVector3(self, excel::IObject::Position());

    const std::string &className = m_kernel_->GetPropertyString(self, excel::IObject::ClassName());
    std::shared_ptr<IRecordManager> pClassRecordManager = m_class_->GetClassRecordManager(className);
    std::shared_ptr<IPropertyManager> pClassPropertyManager = m_class_->GetClassPropertyManager(className);

    std::shared_ptr<IRecord> pRecord(NULL);
    std::shared_ptr<IProperty> pProperty(NULL);
    if (bTable) {
        if (NULL == pClassRecordManager) {
            return -1;
        }

        pRecord = pClassRecordManager->GetElement(propertyName);
        if (NULL == pRecord) {
            return -1;
        }
    } else {
        if (NULL == pClassPropertyManager) {
            return -1;
        }
        pProperty = pClassPropertyManager->GetElement(propertyName);
        if (NULL == pProperty) {
            return -1;
        }
    }

    if (bTable) {
        // upload property can not board to itself
        // if (pRecord->GetUpload())
        {
            if (pRecord->GetPublic()) {
                m_pCellModule->GetCellObjectList(nObjectContainerID, nObjectGroupID, position, valueObject, true, self);
            }

            if (pRecord->GetPrivate()) {
                valueObject.Add(self);
            }
        }
    } else {
        // upload property can not board to itself
        // if (pProperty->GetUpload())
        {
            if (pProperty->GetPublic()) {
                m_pCellModule->GetCellObjectList(nObjectContainerID, nObjectGroupID, position, valueObject, true, self);
            }

            if (pProperty->GetPrivate()) {
                valueObject.Add(self);
            }
        }
    }

    return valueObject.GetCount();
}

int SceneModule::EnterSceneCondition(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) {
    std::vector<SCENE_EVENT_FUNCTOR_PTR>::iterator it = mvEnterSceneConditionCallback.begin();
    for (; it != mvEnterSceneConditionCallback.end(); it++) {
        SCENE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        SCENE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        const int nReason = pFunc->operator()(self, sceneID, groupID, type, argList);
        if (nReason != 0)
            return nReason;
    }
    return 0;
}

int SceneModule::AfterEnterSceneGroup(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) {
    std::vector<SCENE_EVENT_FUNCTOR_PTR>::iterator it = mvAfterEnterSceneCallback.begin();
    for (; it != mvAfterEnterSceneCallback.end(); it++) {
        SCENE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        SCENE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, sceneID, groupID, type, argList);
    }

    return 0;
}

int SceneModule::SceneGroupCreatedEvent(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) {
    std::vector<SCENE_EVENT_FUNCTOR_PTR>::iterator it = mvSceneGroupCreatedCallback.begin();
    for (; it != mvSceneGroupCreatedCallback.end(); it++) {
        SCENE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        SCENE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, sceneID, groupID, type, argList);
    }

    return 0;
}

int SceneModule::SceneGroupDestroyedEvent(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) {
    std::vector<SCENE_EVENT_FUNCTOR_PTR>::iterator it = mvSceneGroupDestroyedCallback.begin();
    for (; it != mvSceneGroupDestroyedCallback.end(); it++) {
        SCENE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        SCENE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, sceneID, groupID, type, argList);
    }

    return 0;
}

int SceneModule::BeforeLeaveSceneGroup(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) {
    std::vector<SCENE_EVENT_FUNCTOR_PTR>::iterator it = mvBeforeLeaveSceneCallback.begin();
    for (; it != mvBeforeLeaveSceneCallback.end(); it++) {
        SCENE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        SCENE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, sceneID, groupID, type, argList);
    }

    return 0;
}

int SceneModule::AfterLeaveSceneGroup(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) {
    std::vector<SCENE_EVENT_FUNCTOR_PTR>::iterator it = mvAfterLeaveSceneCallback.begin();
    for (; it != mvAfterLeaveSceneCallback.end(); it++) {
        SCENE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        SCENE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, sceneID, groupID, type, argList);
    }

    return 0;
}

int SceneModule::OnSwapSceneEvent(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) {
    std::vector<SCENE_EVENT_FUNCTOR_PTR>::iterator it = mvOnSwapSceneCallback.begin();
    for (; it != mvOnSwapSceneCallback.end(); it++) {
        SCENE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        SCENE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, sceneID, groupID, type, argList);
    }
    return 0;
}

int SceneModule::BeforeEnterSceneGroup(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) {

    std::vector<SCENE_EVENT_FUNCTOR_PTR>::iterator it = mvBeforeEnterSceneCallback.begin();
    for (; it != mvBeforeEnterSceneCallback.end(); it++) {
        SCENE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        SCENE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, sceneID, groupID, type, argList);
    }
    return 0;
}

int SceneModule::OnObjectListEnter(const DataList &self, const DataList &argVar) {
    std::vector<OBJECT_ENTER_EVENT_FUNCTOR_PTR>::iterator it = mvObjectEnterCallback.begin();
    for (; it != mvObjectEnterCallback.end(); it++) {
        OBJECT_ENTER_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        OBJECT_ENTER_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, argVar);
    }

    return 0;
}

int SceneModule::OnObjectListEnterFinished(const DataList &self, const DataList &argVar) {
    std::vector<OBJECT_ENTER_EVENT_FUNCTOR_PTR>::iterator it = mvObjectDataFinishedCallBack.begin();
    for (; it != mvObjectDataFinishedCallBack.end(); it++) {
        OBJECT_ENTER_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        OBJECT_ENTER_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, argVar);
    }

    return 0;
}

int SceneModule::OnObjectListLeave(const DataList &self, const DataList &argVar) {
    std::vector<OBJECT_LEAVE_EVENT_FUNCTOR_PTR>::iterator it = mvObjectLeaveCallback.begin();
    for (; it != mvObjectLeaveCallback.end(); it++) {
        OBJECT_LEAVE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        OBJECT_LEAVE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, argVar);
    }

    return 0;
}

int SceneModule::OnPropertyEnter(const DataList &argVar, const Guid &self) {
    std::vector<PROPERTY_ENTER_EVENT_FUNCTOR_PTR>::iterator it = mvPropertyEnterCallback.begin();
    for (; it != mvPropertyEnterCallback.end(); it++) {
        PROPERTY_ENTER_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        PROPERTY_ENTER_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(argVar, self);
    }

    return 0;
}

int SceneModule::OnRecordEnter(const DataList &argVar, const Guid &self) {
    std::vector<RECORD_ENTER_EVENT_FUNCTOR_PTR>::iterator it = mvRecordEnterCallback.begin();
    for (; it != mvRecordEnterCallback.end(); it++) {
        RECORD_ENTER_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        RECORD_ENTER_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(argVar, self);
    }

    return 0;
}

int SceneModule::OnPropertyEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar, const DataList &argVar,
                                 const INT64 reason) {
    std::vector<PROPERTY_SINGLE_EVENT_FUNCTOR_PTR>::iterator it = mvPropertySingleCallback.begin();
    for (; it != mvPropertySingleCallback.end(); it++) {
        PROPERTY_SINGLE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        PROPERTY_SINGLE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, propertyName, oldVar, newVar, argVar, reason);
    }

    return 0;
}

int SceneModule::OnRecordEvent(const Guid &self, const std::string &propertyName, const RECORD_EVENT_DATA &eventData, const SquickData &oldVar,
                               const SquickData &newVar, const DataList &argVar) {
    std::vector<RECORD_SINGLE_EVENT_FUNCTOR_PTR>::iterator it = mvRecordSingleCallback.begin();
    for (; it != mvRecordSingleCallback.end(); it++) {
        RECORD_SINGLE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        RECORD_SINGLE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, propertyName, eventData, oldVar, newVar, argVar);
    }

    return 0;
}

int SceneModule::OnMoveCellEvent(const Guid &self, const int &scene, const int &group, const Guid &fromCell, const Guid &toCell) {
    if (fromCell.IsNull()) {
        // enter a group
        m_pCellModule->OnObjectEntry(self, scene, group, toCell);
    } else if (toCell.IsNull()) {
        // leave a group
        m_pCellModule->OnObjectLeave(self, scene, group, fromCell);
    } else {
        // move between two groups
        m_pCellModule->OnObjectMove(self, scene, group, fromCell, toCell);
    }

    return 0;
}

bool SceneModule::AddAfterEnterAndReadySceneGroupCallBack(const SCENE_EVENT_FUNCTOR_PTR &cb) {
    mvAfterEnterAndReadySceneCallback.push_back(cb);
    return true;
}

int SceneModule::AfterEnterAndReadySceneGroup(const Guid &self, const int sceneID, const int groupID, const int type, const DataList &argList) {
    std::vector<SCENE_EVENT_FUNCTOR_PTR>::iterator it = mvAfterEnterAndReadySceneCallback.begin();
    for (; it != mvAfterEnterAndReadySceneCallback.end(); it++) {
        SCENE_EVENT_FUNCTOR_PTR &pFunPtr = *it;
        SCENE_EVENT_FUNCTOR *pFunc = pFunPtr.get();
        pFunc->operator()(self, sceneID, groupID, type, argList);
    }

    return 0;
}
