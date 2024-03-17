
#include "data_tail_module.h"
#include <squick/core/data_list.h>
#include <squick/core/i_plugin_manager.h>
#include <struct/excel.h>

#if PLATFORM != PLATFORM_WIN
#include <execinfo.h>
#endif

bool DataTailModule::Start() {
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    return true;
}

bool DataTailModule::Destory() { return true; }

bool DataTailModule::Update() { return true; }

bool DataTailModule::AfterStart() {
#ifdef SQUICK_DEBUG_MODE
    m_kernel_->AddClassCallBack(excel::Player::ThisName(), this, &DataTailModule::OnClassObjectEvent);
#endif
    return true;
}

int DataTailModule::OnClassObjectEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var) {
    if (CLASS_OBJECT_EVENT::COE_CREATE_AFTER_ATTACHDATA == classEvent) {
        TrailObjectData(self);
    }

    std::ostringstream stream;
    switch (classEvent) {
    case CLASS_OBJECT_EVENT::COE_CREATE_NODATA:
        stream << " " + className + " COE_CREATE_NODATA";
        break;
    case CLASS_OBJECT_EVENT::COE_CREATE_BEFORE_ATTACHDATA:
        stream << " " + className + " COE_CREATE_BEFORE_ATTACHDATA";
        break;
    case CLASS_OBJECT_EVENT::COE_CREATE_LOADDATA:
        stream << " " + className + " COE_CREATE_LOADDATA";
        break;
    case CLASS_OBJECT_EVENT::COE_CREATE_AFTER_ATTACHDATA:
        stream << " " + className + " COE_CREATE_AFTER_ATTACHDATA";
        break;
    case CLASS_OBJECT_EVENT::COE_CREATE_BEFORE_EFFECT:
        stream << " " + className + " COE_CREATE_BEFORE_EFFECT";
        break;
    case CLASS_OBJECT_EVENT::COE_CREATE_EFFECTDATA:
        stream << " " + className + " COE_CREATE_EFFECTDATA";
        break;
    case CLASS_OBJECT_EVENT::COE_CREATE_AFTER_EFFECT:
        stream << " " + className + " COE_CREATE_AFTER_EFFECT";
        break;
    case CLASS_OBJECT_EVENT::COE_CREATE_HASDATA:
        stream << " " + className + " COE_CREATE_HASDATA";
        break;
    case CLASS_OBJECT_EVENT::COE_CREATE_FINISH:
        stream << " " + className + " COE_CREATE_FINISH";
        break;
    case CLASS_OBJECT_EVENT::COE_CREATE_CLIENT_FINISH:
        stream << " " + className + " COE_CREATE_CLIENT_FINISH";
        break;
    case CLASS_OBJECT_EVENT::COE_BEFOREDESTROY:
        stream << " " + className + " COE_BEFOREDESTROY";
        break;
    case CLASS_OBJECT_EVENT::COE_DESTROY:
        stream << " " + className + " COE_DESTROY";
        break;
    }

    m_log_->LogDebug(self, stream.str());
    return 0;
}

void DataTailModule::StartTrail(const Guid &self) { TrailObjectData(self); }

void DataTailModule::LogObjectData(const Guid &self) {
    std::shared_ptr<IObject> xObject = m_kernel_->GetObject(self);
    if (nullptr == xObject) {
        return;
    }

    std::shared_ptr<IPropertyManager> xPropertyManager = xObject->GetPropertyManager();
    if (nullptr != xPropertyManager) {
        std::shared_ptr<IProperty> xProperty = xPropertyManager->First();
        while (nullptr != xProperty) {
            std::ostringstream stream;

            stream << " Start trail ";
            stream << xProperty->GetKey();
            stream << "==>";
            stream << xProperty->ToString();

            m_log_->LogDebug(self, stream.str(), __FUNCTION__, __LINE__);

            xProperty = xPropertyManager->Next();
        }
    }

    std::shared_ptr<IRecordManager> xRecordManager = xObject->GetRecordManager();
    if (nullptr != xRecordManager) {
        std::shared_ptr<IRecord> xRecord = xRecordManager->First();
        while (nullptr != xRecord) {
            for (int i = 0; i < xRecord->GetRows(); ++i) {
                DataList xDataList;
                bool bRet = xRecord->QueryRow(i, xDataList);
                if (bRet) {
                    std::ostringstream stream;
                    stream << " Start trail Row[" << i << "]";

                    for (int j = 0; j < xDataList.GetCount(); ++j) {
                        stream << " [" << j << "] " << xDataList.ToString(j);
                    }
                    m_log_->LogDebug(stream);
                }
            }

            xRecord = xRecordManager->Next();
        }
    }
}

int DataTailModule::OnObjectPropertyEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar,
                                          const INT64 reason) {
    std::ostringstream stream;

    stream << propertyName;
    stream << ":";
    stream << oldVar.ToString();
    stream << "==>";
    stream << newVar.ToString();

    // m_log_->LogDebug(self, stream.str());

    PrintStackTrace();

    return 0;
}

int DataTailModule::OnObjectRecordEvent(const Guid &self, const RECORD_EVENT_DATA &eventData, const SquickData &oldVar, const SquickData &newVar) {
    std::ostringstream stream;
    std::shared_ptr<IRecord> xRecord = m_kernel_->FindRecord(self, eventData.recordName);
    if (nullptr == xRecord) {
        return 0;
    }

    switch (eventData.nOpType) {
    case RECORD_EVENT_DATA::Add: {
        DataList xDataList;
        bool bRet = xRecord->QueryRow(eventData.row, xDataList);
        if (bRet) {
            stream << xRecord->GetName();
            stream << " Add Row[" << eventData.row << "]";

            for (int j = 0; j < xDataList.GetCount(); ++j) {
                stream << " [" << j << "] " << xDataList.ToString(j);
            }

            // m_log_->LogDebug(self, stream.str());

            PrintStackTrace();
        }
    } break;
    case RECORD_EVENT_DATA::Del: {
        stream << xRecord->GetName();
        stream << " Del Row[" << eventData.row << "]";
        // m_log_->LogDebug(self, stream.str());

        PrintStackTrace();
    } break;
    case RECORD_EVENT_DATA::Swap: {
        stream << xRecord->GetName();
        stream << " Swap Row[" << eventData.row << "] Row[" << eventData.col << "]";
        // m_log_->LogDebug(self, stream.str());
    } break;
    case RECORD_EVENT_DATA::Create:
        break;
    case RECORD_EVENT_DATA::Update: {
        stream << xRecord->GetName();
        stream << " UpData Row[" << eventData.row << "] Col[" << eventData.col << "]";
        stream << oldVar.ToString();
        stream << "==>" << newVar.ToString();
        // m_log_->LogDebug(self, stream.str());

        PrintStackTrace();
    } break;
    case RECORD_EVENT_DATA::Cleared:
        break;
    case RECORD_EVENT_DATA::Sort:
        break;
    default:
        break;
    }

    return 0;
}

int DataTailModule::TrailObjectData(const Guid &self) {
    std::shared_ptr<IObject> xObject = m_kernel_->GetObject(self);
    if (nullptr == xObject) {
        return -1;
    }

    std::shared_ptr<IPropertyManager> xPropertyManager = xObject->GetPropertyManager();
    if (nullptr != xPropertyManager) {
        std::shared_ptr<IProperty> xProperty = xPropertyManager->First();
        while (nullptr != xProperty) {
            m_kernel_->AddPropertyCallBack(self, xProperty->GetKey(), this, &DataTailModule::OnObjectPropertyEvent);

            xProperty = xPropertyManager->Next();
        }
    }

    std::shared_ptr<IRecordManager> xRecordManager = xObject->GetRecordManager();
    if (nullptr != xRecordManager) {
        std::shared_ptr<IRecord> xRecord = xRecordManager->First();
        while (nullptr != xRecord) {
            m_kernel_->AddRecordCallBack(self, xRecord->GetName(), this, &DataTailModule::OnObjectRecordEvent);

            xRecord = xRecordManager->Next();
        }
    }

    return 0;
}

void DataTailModule::PrintStackTrace() { return; }