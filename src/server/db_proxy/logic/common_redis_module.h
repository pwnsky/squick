#pragma once

#include "i_common_redis_module.h"
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/plugin/redis/export.h>
#include <squick/struct/struct.h>

class CommonRedisModule : public ICommonRedisModule {
  public:
    CommonRedisModule(IPluginManager *p);

    virtual bool AfterStart();

  public:
    virtual std::string GetPropertyCacheKey(const std::string &self);
    virtual std::string GetRecordCacheKey(const std::string &self);
    virtual std::string GetFriendCacheKey(const std::string &self);
    virtual std::string GetFriendInviteCacheKey(const std::string &self);
    virtual std::string GetBlockCacheKey(const std::string &self);

    virtual std::string GetTeamCacheKey(const std::string &self);
    virtual std::string GetTeamInviteCacheKey(const std::string &self);

    virtual std::string GetAccountCacheKey(const std::string &account);
    virtual std::string GetTileCacheKey(const int &sceneID);
    virtual std::string GetSceneCacheKey(const int &sceneID);
    virtual std::string GetCellCacheKey(const std::string &strCellID);

    virtual std::shared_ptr<IPropertyManager> NewPropertyManager(const std::string &className);
    virtual std::shared_ptr<IRecordManager> NewRecordManager(const std::string &className);

    virtual std::shared_ptr<IPropertyManager> GetPropertyInfo(const std::string &self, const std::string &className, const bool cache, const bool save,
                                                               std::shared_ptr<IPropertyManager> propertyManager = nullptr);
    virtual std::shared_ptr<IRecordManager> GetRecordInfo(const std::string &self, const std::string &className, const bool cache, const bool save,
                                                           std::shared_ptr<IRecordManager> recordManager = nullptr);
    virtual bool GetRecordInfo(const std::string &self, const std::string &className, rpc::ObjectRecordList *pRecordData, const bool cache,
                               const bool save);

    // support hmset
    virtual bool SavePropertyInfo(const std::string &self, const std::string &propertyName, const std::string &propertyValue);

    virtual bool SavePropertyInfo(const std::string &self, std::shared_ptr<IPropertyManager> pPropertyManager, const bool cache, const bool save,
                                  const int nExpireSecond = -1);
    virtual bool SaveRecordInfo(const std::string &self, std::shared_ptr<IRecordManager> pRecordManager, const bool cache, const bool save,
                                const int nExpireSecond = -1);
    virtual bool SaveRecordInfo(const std::string &self, const rpc::ObjectRecordList &xRecordData, const int nExpireSecond = -1);
    virtual bool GetPropertyList(const std::string &self, const std::vector<std::string> &fields, std::vector<std::string> &values);
    virtual bool GetPropertyList(const std::string &self, std::vector<std::pair<std::string, std::string>> &values);

    virtual INT64 GetPropertyInt(const std::string &self, const std::string &propertyName);
    virtual int GetPropertyInt32(const std::string &self, const std::string &propertyName);
    virtual double GetPropertyFloat(const std::string &self, const std::string &propertyName);
    virtual std::string GetPropertyString(const std::string &self, const std::string &propertyName);
    virtual Guid GetPropertyObject(const std::string &self, const std::string &propertyName);
    virtual Vector2 GetPropertyVector2(const std::string &self, const std::string &propertyName);
    virtual Vector3 GetPropertyVector3(const std::string &self, const std::string &propertyName);

    ////
    static bool ConvertRecordToPB(const std::shared_ptr<IRecord> pRecord, rpc::ObjectRecordBase *pRecordData) {
        pRecordData->set_record_name(pRecord->GetName());

        for (int iRow = 0; iRow < pRecord->GetRows(); iRow++) {
            if (!pRecord->IsUsed(iRow)) {
                continue;
            }

            rpc::RecordAddRowStruct *pRowData = pRecordData->add_row_struct();
            if (!pRowData) {
                continue;
            }

            pRowData->set_row(iRow);

            for (int iCol = 0; iCol < pRecord->GetCols(); iCol++) {
                const int type = pRecord->GetColType(iCol);
                switch (type) {
                case TDATA_INT: {
                    rpc::RecordInt *pPropertyData = pRowData->add_record_int_list();
                    const INT64 xPropertyValue = pRecord->GetInt(iRow, iCol);

                    if (pPropertyData) {
                        pPropertyData->set_col(iCol);
                        pPropertyData->set_row(iRow);
                        pPropertyData->set_data(xPropertyValue);
                    }
                } break;
                case TDATA_FLOAT: {
                    rpc::RecordFloat *pPropertyData = pRowData->add_record_float_list();
                    const double xPropertyValue = pRecord->GetFloat(iRow, iCol);

                    if (pPropertyData) {
                        pPropertyData->set_col(iCol);
                        pPropertyData->set_row(iRow);
                        pPropertyData->set_data(xPropertyValue);
                    }
                } break;
                case TDATA_STRING: {
                    rpc::RecordString *pPropertyData = pRowData->add_record_string_list();
                    const std::string &xPropertyValue = pRecord->GetString(iRow, iCol);

                    if (pPropertyData) {
                        pPropertyData->set_col(iCol);
                        pPropertyData->set_row(iRow);
                        pPropertyData->set_data(xPropertyValue);
                    }
                } break;
                case TDATA_OBJECT: {
                    rpc::RecordObject *pPropertyData = pRowData->add_record_object_list();
                    const Guid xPropertyValue = pRecord->GetObject(iRow, iCol);

                    if (pPropertyData) {
                        pPropertyData->set_col(iCol);
                        pPropertyData->set_row(iRow);
                        *pPropertyData->mutable_data() = INetModule::StructToProtobuf(xPropertyValue);
                    }
                } break;
                case TDATA_VECTOR2: {
                    rpc::RecordVector2 *pPropertyData = pRowData->add_record_vector2_list();
                    const Vector2 xPropertyValue = pRecord->GetVector2(iRow, iCol);

                    if (pPropertyData) {
                        pPropertyData->set_col(iCol);
                        pPropertyData->set_row(iRow);

                        rpc::Vector2 *pVec = pPropertyData->mutable_data();
                        pVec->set_x(xPropertyValue.X());
                        pVec->set_y(xPropertyValue.Y());
                    }
                } break;
                case TDATA_VECTOR3: {
                    rpc::RecordVector3 *pPropertyData = pRowData->add_record_vector3_list();
                    const Vector3 xPropertyValue = pRecord->GetVector3(iRow, iCol);

                    if (pPropertyData) {
                        pPropertyData->set_col(iCol);
                        pPropertyData->set_row(iRow);

                        rpc::Vector3 *pVec = pPropertyData->mutable_data();
                        pVec->set_x(xPropertyValue.X());
                        pVec->set_y(xPropertyValue.Y());
                        pVec->set_z(xPropertyValue.Z());
                    }
                } break;
                default:
                    break;
                }
            }
        }

        return true;
    }
    static bool ConvertPBToRecord(const rpc::ObjectRecordBase &pRecordData, std::shared_ptr<IRecord> pRecord) {
        pRecord->Clear();

        for (int row = 0; row < pRecordData.row_struct_size(); row++) {
            const rpc::RecordAddRowStruct &xAddRowStruct = pRecordData.row_struct(row);

            auto initData = pRecord->GetStartData();
            if (initData) {
                for (int i = 0; i < xAddRowStruct.record_int_list_size(); i++) {
                    const rpc::RecordInt &xPropertyData = xAddRowStruct.record_int_list(i);
                    const int col = xPropertyData.col();
                    const INT64 xPropertyValue = xPropertyData.data();

                    initData->SetInt(col, xPropertyValue);
                }

                for (int i = 0; i < xAddRowStruct.record_float_list_size(); i++) {
                    const rpc::RecordFloat &xPropertyData = xAddRowStruct.record_float_list(i);
                    const int col = xPropertyData.col();
                    const float xPropertyValue = xPropertyData.data();

                    initData->SetFloat(col, xPropertyValue);
                }

                for (int i = 0; i < xAddRowStruct.record_string_list_size(); i++) {
                    const rpc::RecordString &xPropertyData = xAddRowStruct.record_string_list(i);
                    const int col = xPropertyData.col();
                    const std::string &xPropertyValue = xPropertyData.data();

                    initData->SetString(col, xPropertyValue);
                }

                for (int i = 0; i < xAddRowStruct.record_object_list_size(); i++) {
                    const rpc::RecordObject &xPropertyData = xAddRowStruct.record_object_list(i);
                    const int col = xPropertyData.col();
                    const Guid xPropertyValue = INetModule::ProtobufToStruct(xPropertyData.data());

                    initData->SetObject(col, xPropertyValue);
                }

                for (int i = 0; i < xAddRowStruct.record_vector2_list_size(); i++) {
                    const rpc::RecordVector2 &xPropertyData = xAddRowStruct.record_vector2_list(i);
                    const int col = xPropertyData.col();
                    const Vector2 v = INetModule::ProtobufToStruct(xPropertyData.data());

                    initData->SetVector2(col, v);
                }

                for (int i = 0; i < xAddRowStruct.record_vector3_list_size(); i++) {
                    const rpc::RecordVector3 &xPropertyData = xAddRowStruct.record_vector3_list(i);
                    const int col = xPropertyData.col();
                    const Vector3 v = INetModule::ProtobufToStruct(xPropertyData.data());

                    initData->SetVector3(col, v);
                }

                pRecord->AddRow(row, *initData);
            }
        }

        return false;
    }
    // static bool ConvertRecordManagerToPB(const std::shared_ptr<IRecordManager> pRecord, rpc::ObjectRecordList* pRecordData, const bool cache, const
    // bool save);
    static bool ConvertRecordManagerToPB(const std::shared_ptr<IRecordManager> pRecordManager, rpc::ObjectRecordList *pRecordDataList,
                                         const bool cache, const bool save) {
        if (pRecordDataList == nullptr) {
            return false;
        }

        for (std::shared_ptr<IRecord> pRecord = pRecordManager->First(); pRecord != NULL; pRecord = pRecordManager->Next()) {
            if ((cache && pRecord->GetCache()) || (save && pRecord->GetSave())) {
                rpc::ObjectRecordBase *pRecordData = pRecordDataList->add_record_list();
                if (!pRecordData) {
                    continue;
                }

                ConvertRecordToPB(pRecord, pRecordData);
            }
        }

        return true;
    }
    // ConvertPBToRecordManager
    static bool ConvertPBToRecordManager(const rpc::ObjectRecordList &pRecordData, std::shared_ptr<IRecordManager> pRecord) {
        if (pRecord == nullptr) {
            return false;
        }

        for (int i = 0; i < pRecordData.record_list_size(); ++i) {
            const rpc::ObjectRecordBase &xRecordBase = pRecordData.record_list(i);
            std::shared_ptr<IRecord> xRecord = pRecord->GetElement(xRecordBase.record_name());
            if (xRecord) {
                ConvertPBToRecord(xRecordBase, xRecord);
            }
        }

        return true;
    }

    // ConvertPropertyManagerToPB
    static bool ConvertPropertyManagerToPB(const std::shared_ptr<IPropertyManager> pProps, rpc::ObjectPropertyList *pPropertyData, const bool cache,
                                           const bool save) {
        if (pProps) {
            std::shared_ptr<IProperty> xPropert = pProps->First();
            while (xPropert) {
                if ((cache && xPropert->GetCache()) || (save && xPropert->GetSave())) {
                    switch (xPropert->GetType()) {
                    case DATA_TYPE::TDATA_INT: {
                        rpc::PropertyInt *pData = pPropertyData->add_property_int_list();
                        pData->set_property_name(xPropert->GetKey());
                        pData->set_data(xPropert->GetInt());
                    } break;

                    case DATA_TYPE::TDATA_FLOAT: {
                        rpc::PropertyFloat *pData = pPropertyData->add_property_float_list();
                        pData->set_property_name(xPropert->GetKey());
                        pData->set_data(xPropert->GetFloat());
                    } break;

                    case DATA_TYPE::TDATA_OBJECT: {
                        rpc::PropertyObject *pData = pPropertyData->add_property_object_list();
                        pData->set_property_name(xPropert->GetKey());
                        *(pData->mutable_data()) = INetModule::StructToProtobuf(xPropert->GetObject());
                    } break;

                    case DATA_TYPE::TDATA_STRING: {
                        rpc::PropertyString *pData = pPropertyData->add_property_string_list();
                        pData->set_property_name(xPropert->GetKey());
                        pData->set_data(xPropert->GetString());
                    } break;

                    case DATA_TYPE::TDATA_VECTOR2: {
                        rpc::PropertyVector2 *pData = pPropertyData->add_property_vector2_list();
                        pData->set_property_name(xPropert->GetKey());
                        *(pData->mutable_data()) = INetModule::StructToProtobuf(xPropert->GetVector2());
                    } break;

                    case DATA_TYPE::TDATA_VECTOR3: {
                        rpc::PropertyVector3 *pData = pPropertyData->add_property_vector3_list();
                        pData->set_property_name(xPropert->GetKey());
                        *(pData->mutable_data()) = INetModule::StructToProtobuf(xPropert->GetVector3());
                    } break;
                    default:
                        break;
                    }
                }
                xPropert = pProps->Next();
            }
        }

        return false;
    }

    // ConvertPBToPropertyManager
    static bool ConvertPBToPropertyManager(const rpc::ObjectPropertyList &pPropertyData, std::shared_ptr<IPropertyManager> pProps) {
        if (pProps) {
            for (int i = 0; i < pPropertyData.property_int_list_size(); ++i) {
                const rpc::PropertyInt &xData = pPropertyData.property_int_list(i);

                if (pProps->ExistElement(xData.property_name())) {
                    pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_INT);
                }

                pProps->SetPropertyInt(xData.property_name(), xData.data());
            }

            for (int i = 0; i < pPropertyData.property_float_list_size(); ++i) {
                const rpc::PropertyFloat &xData = pPropertyData.property_float_list(i);

                if (pProps->ExistElement(xData.property_name())) {
                    pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_FLOAT);
                }

                pProps->SetPropertyFloat(xData.property_name(), xData.data());
            }

            for (int i = 0; i < pPropertyData.property_string_list_size(); ++i) {
                const rpc::PropertyString &xData = pPropertyData.property_string_list(i);

                if (pProps->ExistElement(xData.property_name())) {
                    pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_STRING);
                }

                pProps->SetPropertyString(xData.property_name(), xData.data());
            }

            for (int i = 0; i < pPropertyData.property_object_list_size(); ++i) {
                const rpc::PropertyObject &xData = pPropertyData.property_object_list(i);

                if (pProps->ExistElement(xData.property_name())) {
                    pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_OBJECT);
                }

                pProps->SetPropertyObject(xData.property_name(), Guid(xData.data().svrid(), xData.data().index()));
            }

            for (int i = 0; i < pPropertyData.property_vector2_list_size(); ++i) {
                const rpc::PropertyVector2 &xData = pPropertyData.property_vector2_list(i);

                if (pProps->ExistElement(xData.property_name())) {
                    pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_VECTOR2);
                }

                pProps->SetPropertyVector2(xData.property_name(), Vector2(xData.data().x(), xData.data().y()));
            }

            for (int i = 0; i < pPropertyData.property_vector3_list_size(); ++i) {
                const rpc::PropertyVector3 &xData = pPropertyData.property_vector3_list(i);

                if (pProps->ExistElement(xData.property_name())) {
                    pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_VECTOR3);
                }

                pProps->SetPropertyVector3(xData.property_name(), Vector3(xData.data().x(), xData.data().y(), xData.data().z()));
            }
        }
        return true;
    }

  protected:
    // support hmset
    virtual bool ConvertVectorToPropertyManager(std::vector<std::string> &vKeyList, std::vector<std::string> &vValueList,
                                                std::shared_ptr<IPropertyManager> pPropertyManager, const bool cache, const bool save);
    virtual bool ConvertVectorToRecordManager(std::vector<std::string> &vKeyList, std::vector<std::string> &vValueList,
                                              std::shared_ptr<IRecordManager> pRecordManager, const bool cache, const bool save);

    // support hmset
    virtual bool ConvertPropertyManagerToVector(std::shared_ptr<IPropertyManager> pPropertyManager, std::vector<std::string> &vKeyList,
                                                std::vector<std::string> &vValueList, const bool cache, const bool save);
    virtual bool ConvertRecordManagerToVector(std::shared_ptr<IRecordManager> pRecordManager, std::vector<std::string> &vKeyList,
                                              std::vector<std::string> &vValueList, const bool cache, const bool save);

    virtual std::shared_ptr<IPropertyManager> GetPropertyInfo(const std::string &self, const std::string &className, std::vector<std::string> &vKeyList,
                                                               std::vector<std::string> &vValueList, const bool cache, const bool save,
                                                               std::shared_ptr<IPropertyManager> propertyManager = nullptr);
    virtual std::shared_ptr<IRecordManager> GetRecordInfo(const std::string &self, const std::string &className, std::vector<std::string> &vKeyList,
                                                           std::vector<std::string> &vValueList, const bool cache, const bool save,
                                                           std::shared_ptr<IRecordManager> recordManager = nullptr);

  protected:
    IKernelModule *m_kernel_;
    IClassModule *m_class_;
    IRedisModule *m_redis_;
    IElementModule *m_element_;
    ILogModule *m_log_;
};
