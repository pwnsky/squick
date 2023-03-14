#pragma once

#include <squick/struct/struct.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/redis/export.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/net/i_net_module.h>
#include "i_common_redis_module.h"



class CommonRedisModule : public ICommonRedisModule
{
public:
    CommonRedisModule(IPluginManager* p);

    virtual bool AfterStart();

public:
    virtual std::string GetPropertyCacheKey(const std::string& self);
	virtual std::string GetRecordCacheKey(const std::string& self);
	virtual std::string GetFriendCacheKey(const std::string& self);
	virtual std::string GetFriendInviteCacheKey(const std::string& self);
	virtual std::string GetBlockCacheKey(const std::string& self);

	virtual std::string GetTeamCacheKey(const std::string& self);
	virtual std::string GetTeamInviteCacheKey(const std::string& self);

	virtual std::string GetAccountCacheKey(const std::string& account);
	virtual std::string GetTileCacheKey(const int& sceneID);
	virtual std::string GetSceneCacheKey(const int& sceneID);
	virtual std::string GetCellCacheKey(const std::string& strCellID);

    virtual SQUICK_SHARE_PTR<IPropertyManager> NewPropertyManager(const std::string& className);
    virtual SQUICK_SHARE_PTR<IRecordManager> NewRecordManager(const std::string& className);

	virtual SQUICK_SHARE_PTR<IPropertyManager> GetPropertyInfo(const std::string& self, const std::string& className, const bool cache, const bool save, SQUICK_SHARE_PTR<IPropertyManager> propertyManager = nullptr);
	virtual SQUICK_SHARE_PTR<IRecordManager> GetRecordInfo(const std::string& self, const std::string& className, const bool cache, const bool save, SQUICK_SHARE_PTR<IRecordManager> recordManager = nullptr);
	virtual bool GetRecordInfo(const std::string& self, const std::string& className, SquickStruct::ObjectRecordList* pRecordData, const bool cache, const bool save);

	//support hmset
	virtual bool SavePropertyInfo(const std::string& self, const std::string& propertyName, const std::string& propertyValue);

	virtual bool SavePropertyInfo(const std::string& self, SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager, const bool cache, const bool save, const int nExpireSecond = -1);
	virtual bool SaveRecordInfo(const std::string& self, SQUICK_SHARE_PTR<IRecordManager> pRecordManager, const bool cache, const bool save, const int nExpireSecond = -1);
	virtual bool SaveRecordInfo(const std::string& self, const SquickStruct::ObjectRecordList& xRecordData, const int nExpireSecond = -1);
	virtual bool GetPropertyList(const std::string& self, const std::vector<std::string>& fields, std::vector<std::string>& values);
	virtual bool GetPropertyList(const std::string& self, std::vector<std::pair<std::string, std::string>>& values);

	virtual INT64 GetPropertyInt(const std::string& self, const std::string& propertyName);
	virtual int GetPropertyInt32(const std::string& self, const std::string& propertyName);
	virtual double GetPropertyFloat(const std::string& self, const std::string& propertyName);
	virtual std::string GetPropertyString(const std::string& self, const std::string& propertyName);
	virtual Guid GetPropertyObject(const std::string& self, const std::string& propertyName);
	virtual Vector2 GetPropertyVector2(const std::string& self, const std::string& propertyName);
	virtual Vector3 GetPropertyVector3(const std::string& self, const std::string& propertyName);

	////
	static bool ConvertRecordToPB(const SQUICK_SHARE_PTR<IRecord> pRecord, SquickStruct::ObjectRecordBase* pRecordData) {
	pRecordData->set_record_name(pRecord->GetName());

	for (int iRow = 0; iRow < pRecord->GetRows(); iRow++)
	{
		if (!pRecord->IsUsed(iRow))
		{
			continue;
		}

		SquickStruct::RecordAddRowStruct* pRowData = pRecordData->add_row_struct();
		if (!pRowData)
		{
			continue;
		}

		pRowData->set_row(iRow);

		for (int iCol = 0; iCol < pRecord->GetCols(); iCol++)
		{
			const int type = pRecord->GetColType(iCol);
			switch (type)
			{
			case TDATA_INT:
			{
				SquickStruct::RecordInt* pPropertyData = pRowData->add_record_int_list();
				const INT64 xPropertyValue = pRecord->GetInt(iRow, iCol);

				if (pPropertyData)
				{
					pPropertyData->set_col(iCol);
					pPropertyData->set_row(iRow);
					pPropertyData->set_data(xPropertyValue);
				}
			}
			break;
			case TDATA_FLOAT:
			{
				SquickStruct::RecordFloat* pPropertyData = pRowData->add_record_float_list();
				const double xPropertyValue = pRecord->GetFloat(iRow, iCol);

				if (pPropertyData)
				{
					pPropertyData->set_col(iCol);
					pPropertyData->set_row(iRow);
					pPropertyData->set_data(xPropertyValue);
				}
			}
			break;
			case TDATA_STRING:
			{
				SquickStruct::RecordString* pPropertyData = pRowData->add_record_string_list();
				const std::string& xPropertyValue = pRecord->GetString(iRow, iCol);

				if (pPropertyData)
				{
					pPropertyData->set_col(iCol);
					pPropertyData->set_row(iRow);
					pPropertyData->set_data(xPropertyValue);

				}
			}
			break;
			case TDATA_OBJECT:
			{
				SquickStruct::RecordObject* pPropertyData = pRowData->add_record_object_list();
				const Guid xPropertyValue = pRecord->GetObject(iRow, iCol);

				if (pPropertyData)
				{
					pPropertyData->set_col(iCol);
					pPropertyData->set_row(iRow);
					*pPropertyData->mutable_data() = INetModule::StructToProtobuf(xPropertyValue);
				}
			}
			break;
			case TDATA_VECTOR2:
			{
				SquickStruct::RecordVector2* pPropertyData = pRowData->add_record_vector2_list();
				const Vector2 xPropertyValue = pRecord->GetVector2(iRow, iCol);

				if (pPropertyData)
				{
					pPropertyData->set_col(iCol);
					pPropertyData->set_row(iRow);

					SquickStruct::Vector2* pVec = pPropertyData->mutable_data();
					pVec->set_x(xPropertyValue.X());
					pVec->set_y(xPropertyValue.Y());
				}
			}
			break;
			case TDATA_VECTOR3:
			{
				SquickStruct::RecordVector3* pPropertyData = pRowData->add_record_vector3_list();
				const Vector3 xPropertyValue = pRecord->GetVector3(iRow, iCol);

				if (pPropertyData)
				{
					pPropertyData->set_col(iCol);
					pPropertyData->set_row(iRow);

					SquickStruct::Vector3* pVec = pPropertyData->mutable_data();
					pVec->set_x(xPropertyValue.X());
					pVec->set_y(xPropertyValue.Y());
					pVec->set_z(xPropertyValue.Z());
				}
			}
			break;
			default:
				break;
			}
		}
	}

	return true;
}
	static bool ConvertPBToRecord(const SquickStruct::ObjectRecordBase& pRecordData, SQUICK_SHARE_PTR<IRecord> pRecord) {
	pRecord->Clear();

	for (int row = 0; row < pRecordData.row_struct_size(); row++)
	{
		const SquickStruct::RecordAddRowStruct& xAddRowStruct = pRecordData.row_struct(row);

		auto initData = pRecord->GetStartData();
		if (initData)
		{
			for (int i = 0; i < xAddRowStruct.record_int_list_size(); i++)
			{
				const SquickStruct::RecordInt& xPropertyData = xAddRowStruct.record_int_list(i);
				const int col = xPropertyData.col();
				const INT64 xPropertyValue = xPropertyData.data();

				initData->SetInt(col, xPropertyValue);
			}

			for (int i = 0; i < xAddRowStruct.record_float_list_size(); i++)
			{
				const SquickStruct::RecordFloat& xPropertyData = xAddRowStruct.record_float_list(i);
				const int col = xPropertyData.col();
				const float xPropertyValue = xPropertyData.data();

				initData->SetFloat(col, xPropertyValue);
			}

			for (int i = 0; i < xAddRowStruct.record_string_list_size(); i++)
			{
				const SquickStruct::RecordString& xPropertyData = xAddRowStruct.record_string_list(i);
				const int col = xPropertyData.col();
				const std::string& xPropertyValue = xPropertyData.data();

				initData->SetString(col, xPropertyValue);
			}

			for (int i = 0; i < xAddRowStruct.record_object_list_size(); i++)
			{
				const SquickStruct::RecordObject& xPropertyData = xAddRowStruct.record_object_list(i);
				const int col = xPropertyData.col();
				const Guid xPropertyValue = INetModule::ProtobufToStruct(xPropertyData.data());

				initData->SetObject(col, xPropertyValue);
			}

			for (int i = 0; i < xAddRowStruct.record_vector2_list_size(); i++)
			{
				const SquickStruct::RecordVector2& xPropertyData = xAddRowStruct.record_vector2_list(i);
				const int col = xPropertyData.col();
				const Vector2 v = INetModule::ProtobufToStruct(xPropertyData.data());

				initData->SetVector2(col, v);
			}

			for (int i = 0; i < xAddRowStruct.record_vector3_list_size(); i++)
			{
				const SquickStruct::RecordVector3& xPropertyData = xAddRowStruct.record_vector3_list(i);
				const int col = xPropertyData.col();
				const Vector3 v = INetModule::ProtobufToStruct(xPropertyData.data());

				initData->SetVector3(col, v);
			}

			pRecord->AddRow(row, *initData);
		}
	}

	return false;
}
	//static bool ConvertRecordManagerToPB(const SQUICK_SHARE_PTR<IRecordManager> pRecord, SquickStruct::ObjectRecordList* pRecordData, const bool cache, const bool save);
	static bool ConvertRecordManagerToPB(const SQUICK_SHARE_PTR<IRecordManager> pRecordManager, SquickStruct::ObjectRecordList * pRecordDataList, const bool cache, const bool save){
	if (pRecordDataList == nullptr )
	{
		return false;
	}

	for (SQUICK_SHARE_PTR<IRecord> pRecord = pRecordManager->First(); pRecord != NULL; pRecord = pRecordManager->Next())
	{
		if ((cache && pRecord->GetCache()) || (save && pRecord->GetSave()))
		{
			SquickStruct::ObjectRecordBase *pRecordData = pRecordDataList->add_record_list();
			if (!pRecordData)
			{
				continue;
			}

			ConvertRecordToPB(pRecord, pRecordData);
		}
	}

	return true;
}
// ConvertPBToRecordManager
	static bool ConvertPBToRecordManager(const SquickStruct::ObjectRecordList& pRecordData, SQUICK_SHARE_PTR<IRecordManager> pRecord){
	if (pRecord == nullptr)
	{
		return false;
	}

	for (int i = 0; i < pRecordData.record_list_size(); ++i)
	{
		const SquickStruct::ObjectRecordBase& xRecordBase = pRecordData.record_list(i);
		SQUICK_SHARE_PTR<IRecord> xRecord = pRecord->GetElement(xRecordBase.record_name());
		if (xRecord)
		{
			ConvertPBToRecord(xRecordBase, xRecord);
		}
	}

	return true;
	}

	// ConvertPropertyManagerToPB
	static bool ConvertPropertyManagerToPB(const SQUICK_SHARE_PTR<IPropertyManager> pProps, SquickStruct::ObjectPropertyList* pPropertyData, const bool cache, const bool save) {
	if (pProps)
	{
		SQUICK_SHARE_PTR<IProperty> xPropert = pProps->First();
		while (xPropert)
		{
			if ((cache && xPropert->GetCache()) || (save && xPropert->GetSave()))
			{
				switch (xPropert->GetType())
				{
				case DATA_TYPE::TDATA_INT:
				{
					SquickStruct::PropertyInt* pData = pPropertyData->add_property_int_list();
					pData->set_property_name(xPropert->GetKey());
					pData->set_data(xPropert->GetInt());
				}
				break;

				case DATA_TYPE::TDATA_FLOAT:
				{
					SquickStruct::PropertyFloat* pData = pPropertyData->add_property_float_list();
					pData->set_property_name(xPropert->GetKey());
					pData->set_data(xPropert->GetFloat());
				}
				break;

				case DATA_TYPE::TDATA_OBJECT:
				{
					SquickStruct::PropertyObject* pData = pPropertyData->add_property_object_list();
					pData->set_property_name(xPropert->GetKey());
					*(pData->mutable_data()) = INetModule::StructToProtobuf(xPropert->GetObject());
				}
				break;

				case DATA_TYPE::TDATA_STRING:
				{
					SquickStruct::PropertyString* pData = pPropertyData->add_property_string_list();
					pData->set_property_name(xPropert->GetKey());
					pData->set_data(xPropert->GetString());
				}
				break;

				case DATA_TYPE::TDATA_VECTOR2:
				{
					SquickStruct::PropertyVector2* pData = pPropertyData->add_property_vector2_list();
					pData->set_property_name(xPropert->GetKey());
					*(pData->mutable_data()) = INetModule::StructToProtobuf(xPropert->GetVector2());
				}
				break;

				case DATA_TYPE::TDATA_VECTOR3:
				{
					SquickStruct::PropertyVector3* pData = pPropertyData->add_property_vector3_list();
					pData->set_property_name(xPropert->GetKey());
					*(pData->mutable_data()) = INetModule::StructToProtobuf(xPropert->GetVector3());
				}
				break;
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
	static bool ConvertPBToPropertyManager(const SquickStruct::ObjectPropertyList& pPropertyData, SQUICK_SHARE_PTR<IPropertyManager> pProps) {
	if (pProps)
	{
		for (int i = 0; i < pPropertyData.property_int_list_size(); ++i)
		{
			const SquickStruct::PropertyInt& xData = pPropertyData.property_int_list(i);

			if (pProps->ExistElement(xData.property_name()))
			{
				pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_INT);
			}

			pProps->SetPropertyInt(xData.property_name(), xData.data());
		}

		for (int i = 0; i < pPropertyData.property_float_list_size(); ++i)
		{
			const SquickStruct::PropertyFloat& xData = pPropertyData.property_float_list(i);

			if (pProps->ExistElement(xData.property_name()))
			{
				pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_FLOAT);
			}

			pProps->SetPropertyFloat(xData.property_name(), xData.data());
		}

		for (int i = 0; i < pPropertyData.property_string_list_size(); ++i)
		{
			const SquickStruct::PropertyString& xData = pPropertyData.property_string_list(i);

			if (pProps->ExistElement(xData.property_name()))
			{
				pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_STRING);
			}

			pProps->SetPropertyString(xData.property_name(), xData.data());
		}

		for (int i = 0; i < pPropertyData.property_object_list_size(); ++i)
		{
			const SquickStruct::PropertyObject& xData = pPropertyData.property_object_list(i);

			if (pProps->ExistElement(xData.property_name()))
			{
				pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_OBJECT);
			}

			pProps->SetPropertyObject(xData.property_name(), Guid(xData.data().svrid(), xData.data().index()));
		}

		for (int i = 0; i < pPropertyData.property_vector2_list_size(); ++i)
		{
			const SquickStruct::PropertyVector2& xData = pPropertyData.property_vector2_list(i);

			if (pProps->ExistElement(xData.property_name()))
			{
				pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_VECTOR2);
			}

			pProps->SetPropertyVector2(xData.property_name(), Vector2(xData.data().x(), xData.data().y()));
		}

		for (int i = 0; i < pPropertyData.property_vector3_list_size(); ++i)
		{
			const SquickStruct::PropertyVector3& xData = pPropertyData.property_vector3_list(i);

			if (pProps->ExistElement(xData.property_name()))
			{
				pProps->AddProperty(pProps->Self(), xData.property_name(), DATA_TYPE::TDATA_VECTOR3);
			}

			pProps->SetPropertyVector3(xData.property_name(), Vector3(xData.data().x(), xData.data().y(), xData.data().z()));
		}
	}
	return true;
}

protected:
	//support hmset
	virtual bool ConvertVectorToPropertyManager(std::vector<std::string>& vKeyList, std::vector<std::string>& vValueList, SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager, const bool cache, const bool save);
	virtual bool ConvertVectorToRecordManager(std::vector<std::string>& vKeyList, std::vector<std::string>& vValueList, SQUICK_SHARE_PTR<IRecordManager> pRecordManager, const bool cache, const bool save);

	//support hmset
	virtual bool ConvertPropertyManagerToVector(SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager, std::vector<std::string>& vKeyList, std::vector<std::string>& vValueList, const bool cache, const bool save);
	virtual bool ConvertRecordManagerToVector(SQUICK_SHARE_PTR<IRecordManager> pRecordManager, std::vector<std::string>& vKeyList, std::vector<std::string>& vValueList, const bool cache, const bool save);

	virtual SQUICK_SHARE_PTR<IPropertyManager> GetPropertyInfo(const std::string& self, const std::string& className, std::vector<std::string>& vKeyList, std::vector<std::string>& vValueList, const bool cache, const bool save, SQUICK_SHARE_PTR<IPropertyManager> propertyManager = nullptr);
	virtual SQUICK_SHARE_PTR<IRecordManager> GetRecordInfo(const std::string& self, const std::string& className, std::vector<std::string>& vKeyList, std::vector<std::string>& vValueList, const bool cache, const bool save, SQUICK_SHARE_PTR<IRecordManager> recordManager = nullptr);

protected:
	IKernelModule* m_pKernelModule;
	IClassModule* m_pLogicClassModule;
    INoSqlModule* m_pNoSqlModule;
	IElementModule* m_pElementModule;
	ILogModule* m_pLogModule;
};

