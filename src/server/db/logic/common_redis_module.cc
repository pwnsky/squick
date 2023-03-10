

#include "common_redis_module.h"
#include <squick/core/property_manager.h>
#include <squick/core/record_manager.h>


CommonRedisModule::CommonRedisModule(IPluginManager * p)
{
	pPluginManager = p;
}

std::string CommonRedisModule::GetPropertyCacheKey(const std::string& self)
{
    return self + "_ObjectProperty";
}

std::string CommonRedisModule::GetRecordCacheKey(const std::string& self)
{
    return self  + "_ObjectRecord";
}

std::string CommonRedisModule::GetFriendCacheKey(const std::string& self)
{
    return self + "_ObjectFriend";
}

std::string CommonRedisModule::GetFriendInviteCacheKey(const std::string& self)
{
    return self + "_ObjectFriendInvite";
}

std::string CommonRedisModule::GetBlockCacheKey(const std::string& self)
{
    return self + "_ObjectBlock";
}

std::string CommonRedisModule::GetTeamCacheKey(const std::string & self)
{
	return self + "_Team";
}

std::string CommonRedisModule::GetTeamInviteCacheKey(const std::string & self)
{
	return self + "_TeamInvite";
}

std::string CommonRedisModule::GetAccountCacheKey(const std::string & account)
{
	return account + "_AccountInfo";
}

std::string CommonRedisModule::GetTileCacheKey(const int & sceneID)
{
	return lexical_cast<std::string>(sceneID) + "_TileInfo";
}

std::string CommonRedisModule::GetSceneCacheKey(const int & sceneID)
{
	return lexical_cast<std::string>(sceneID) + "_SceneProps";
}

std::string CommonRedisModule::GetCellCacheKey(const std::string & strCellID)
{
	return strCellID + "_CellInfo";
}

bool CommonRedisModule::AfterStart()
{
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	m_pLogicClassModule = pPluginManager->FindModule<IClassModule>();
    m_pNoSqlModule = pPluginManager->FindModule<INoSqlModule>();
	m_pElementModule = pPluginManager->FindModule<IElementModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();

    return true;
}

SQUICK_SHARE_PTR<IPropertyManager> CommonRedisModule::NewPropertyManager(const std::string& className)
{
    SQUICK_SHARE_PTR<IPropertyManager> pStaticClassPropertyManager = m_pLogicClassModule->GetClassPropertyManager(className);
    if (pStaticClassPropertyManager)
    {
        Guid ident;
        SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager(SQUICK_NEW PropertyManager(ident));

        SQUICK_SHARE_PTR<IProperty> pStaticConfigPropertyInfo = pStaticClassPropertyManager->First();
        while (pStaticConfigPropertyInfo)
        {
			if (pStaticConfigPropertyInfo->GetSave() || pStaticConfigPropertyInfo->GetCache())
			{
				SQUICK_SHARE_PTR<IProperty> xProperty = pPropertyManager->AddProperty(ident, pStaticConfigPropertyInfo->GetKey(), pStaticConfigPropertyInfo->GetType());

				xProperty->SetPublic(pStaticConfigPropertyInfo->GetPublic());
				xProperty->SetPrivate(pStaticConfigPropertyInfo->GetPrivate());
				xProperty->SetSave(pStaticConfigPropertyInfo->GetSave());
				xProperty->SetCache(pStaticConfigPropertyInfo->GetCache());
				xProperty->SetRef(pStaticConfigPropertyInfo->GetRef());
			}

            pStaticConfigPropertyInfo = pStaticClassPropertyManager->Next();
        }

        return pPropertyManager;
    }

    return SQUICK_SHARE_PTR<IPropertyManager>(NULL);
}

SQUICK_SHARE_PTR<IRecordManager> CommonRedisModule::NewRecordManager(const std::string& className)
{
    SQUICK_SHARE_PTR<IRecordManager> pStaticClassRecordManager = m_pLogicClassModule->GetClassRecordManager(className);
    if (pStaticClassRecordManager)
    {
        Guid ident;
        SQUICK_SHARE_PTR<IRecordManager> pRecordManager(SQUICK_NEW RecordManager(ident));

        SQUICK_SHARE_PTR<IRecord> pConfigRecordInfo = pStaticClassRecordManager->First();
        while (pConfigRecordInfo)
        {
			if (pConfigRecordInfo->GetSave() || pConfigRecordInfo->GetCache())
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

			}

            pConfigRecordInfo = pStaticClassRecordManager->Next();
        }

        return pRecordManager;
    }

    return SQUICK_SHARE_PTR<IRecordManager>(NULL);
}

SQUICK_SHARE_PTR<IPropertyManager> CommonRedisModule::GetPropertyInfo(const std::string& self, const std::string& className, std::vector<std::string>& vKeyCacheList, std::vector<std::string>& vValueCacheList, const bool cache, const bool save, SQUICK_SHARE_PTR<IPropertyManager> propertyManager)
{
	//TODO optimize
	SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager = propertyManager;
	if (pPropertyManager == nullptr)
	{
		pPropertyManager = NewPropertyManager(className);
		if (!pPropertyManager)
		{
			return nullptr;
		}
	}

	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
    if (!pDriver)
    {
        return nullptr;
    }

	//TODO
	//just run this function one time
	SQUICK_SHARE_PTR<IProperty> xProperty = pPropertyManager->First();
	while (xProperty)
	{
		if ((cache && xProperty->GetCache()) || (save && xProperty->GetSave()))
		{
			vKeyCacheList.push_back(xProperty->GetKey());
		}

		xProperty = pPropertyManager->Next();
	}

	//cache
	std::string strCacheKey = GetPropertyCacheKey(self);
    if (!pDriver->HMGET(strCacheKey, vKeyCacheList, vValueCacheList))
    {
        return nullptr;
    }

	if (vKeyCacheList.size() == vValueCacheList.size())
	{
		ConvertVectorToPropertyManager(vKeyCacheList, vValueCacheList, pPropertyManager, cache, save);

		return pPropertyManager;
	}

	return nullptr;
}

SQUICK_SHARE_PTR<IRecordManager> CommonRedisModule::GetRecordInfo(const std::string& self, const std::string& className, std::vector<std::string>& vKeyCacheList, std::vector<std::string>& vValueCacheList, const bool cache, const bool save, SQUICK_SHARE_PTR<IRecordManager> recordManager)
{
	SQUICK_SHARE_PTR<IRecordManager> pRecordManager = recordManager;
	if (pRecordManager == nullptr)
	{
		pRecordManager = NewRecordManager(className);
		if (!pRecordManager)
		{
			return nullptr;
		}
	}

	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
    if (!pDriver)
    {
        return nullptr;
    }

	//TODO
	//just run this function one time
	SQUICK_SHARE_PTR<IRecord> xRecord = pRecordManager->First();
	while (xRecord)
	{
		if ((cache && xRecord->GetCache()) || (save && xRecord->GetSave()))
		{
			vKeyCacheList.push_back(xRecord->GetName());
		}

		xRecord = pRecordManager->Next();
	}

	//cache
	std::string strCacheKey = GetRecordCacheKey(self);
	if (!pDriver->HMGET(strCacheKey, vKeyCacheList, vValueCacheList))
	{
		return nullptr;
	}

	if (vKeyCacheList.size() == vValueCacheList.size())
	{
		ConvertVectorToRecordManager(vKeyCacheList, vValueCacheList, pRecordManager, cache, save);
		return pRecordManager;
	}

	return nullptr;
}

SQUICK_SHARE_PTR<IPropertyManager> CommonRedisModule::GetPropertyInfo(const std::string & self, const std::string & className, const bool cache, const bool save, SQUICK_SHARE_PTR<IPropertyManager> propertyManager)
{
	std::vector<std::string> vKeyCacheList;
	std::vector<std::string> vValueCacheList;

	return GetPropertyInfo(self, className, vKeyCacheList, vValueCacheList, cache, save, propertyManager);
}

SQUICK_SHARE_PTR<IRecordManager> CommonRedisModule::GetRecordInfo(const std::string & self, const std::string & className, const bool cache, const bool save, SQUICK_SHARE_PTR<IRecordManager> recordManager)
{
	std::vector<std::string> vKeyCacheList;
	std::vector<std::string> vValueCacheList;

	return GetRecordInfo(self, className, vKeyCacheList, vValueCacheList, cache, save, recordManager);
}

bool CommonRedisModule::GetRecordInfo(const std::string & self, const std::string & className, SquickStruct::ObjectRecordList * pRecordDataList, const bool cache, const bool save)
{
	*(pRecordDataList->mutable_player_id()) = INetModule::StructToProtobuf(Guid(self));

	std::vector<std::string> vKeyCacheList;
	std::vector<std::string> vValueCacheList;
	SQUICK_SHARE_PTR<IRecordManager> xRecordManager = GetRecordInfo(self, className, vKeyCacheList, vValueCacheList, cache, save);
	if (xRecordManager && vKeyCacheList.size() == vValueCacheList.size())
	{
		if (ConvertRecordManagerToPB(xRecordManager, pRecordDataList, cache, save))
		{
			return true;
		}
	}

	return false;
}

bool CommonRedisModule::SavePropertyInfo(const std::string& self, SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager, const bool cache, const bool save, const int nExpireSecond)
{
    if (!pPropertyManager)
    {
        return false;
    }

	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
    if (!pDriver)
    {
        return false;
    }

	std::vector<std::string> vKeyList;
	std::vector<std::string> vValueList;
    if (!ConvertPropertyManagerToVector(pPropertyManager, vKeyList, vValueList, cache, save))
    {
        return false;
    }

	if (vKeyList.size() != vValueList.size())
	{
		return false;
	}

	std::string strKey= GetPropertyCacheKey(self);

    if (!pDriver->HMSET(strKey, vKeyList, vValueList))
    {
        return false;
    }

	if (nExpireSecond > 0)
	{
		pDriver->EXPIRE(strKey, nExpireSecond);
	}

    return true;
}

bool CommonRedisModule::SaveRecordInfo(const std::string& self, SQUICK_SHARE_PTR<IRecordManager> pRecordManager, const bool cache, const bool save, const int nExpireSecond)
{
    if (!pRecordManager)
    {
        return false;
    }

	SquickStruct::ObjectRecordList xRecordList;
	if (ConvertRecordManagerToPB(pRecordManager, &xRecordList, cache, save))
	{
		return SaveRecordInfo(self, xRecordList, nExpireSecond);
	}

    return false;
}

bool CommonRedisModule::SaveRecordInfo(const std::string & self, const SquickStruct::ObjectRecordList& xRecordData, const int nExpireSecond)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return false;
	}

	std::vector<std::string> vKeyList;
	std::vector<std::string> vValueList;
	
	for (int i = 0; i < xRecordData.record_list_size(); ++i)
	{
		const SquickStruct::ObjectRecordBase& xRecord = xRecordData.record_list(i);

		std::string strValue;
		if (!xRecord.SerializeToString(&strValue))
		{
			continue;
		}

		vKeyList.push_back(xRecord.record_name());
		vValueList.push_back(strValue);
	}


	if (vKeyList.size() != vValueList.size())
	{
		return false;
	}

	std::string strKey = GetRecordCacheKey(self);
	if (!pDriver->HMSET(strKey, vKeyList, vValueList))
	{
		return false;
	}

	if (nExpireSecond > 0)
	{
		pDriver->EXPIRE(strKey, nExpireSecond);
	}

	return true;
}

bool CommonRedisModule::GetPropertyList(const std::string& self, const std::vector<std::string>& fields, std::vector<std::string>& values)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return false;
	}

	std::string strCacheKey = GetPropertyCacheKey(self);
	if (pDriver->HMGET(strCacheKey, fields, values))
	{
		return true;
	}


	return false;
}

INT64 CommonRedisModule::GetPropertyInt(const std::string & self, const std::string & propertyName)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return 0;
	}

	std::string strValue;
	std::string strCacheKey = GetPropertyCacheKey(self);
	if (!pDriver->HGET(strCacheKey, propertyName, strValue))
	{
		return 0;
	}

	return lexical_cast<INT64>(strValue);
}

int CommonRedisModule::GetPropertyInt32(const std::string & self, const std::string & propertyName)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return 0;
	}

	std::string strValue;
	std::string strCacheKey = GetPropertyCacheKey(self);
	if (!pDriver->HGET(strCacheKey, propertyName, strValue))
	{
		return 0;
	}

	return lexical_cast<INT64>(strValue);
}

double CommonRedisModule::GetPropertyFloat(const std::string & self, const std::string & propertyName)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return 0;
	}

	std::string strValue;
	std::string strCacheKey = GetPropertyCacheKey(self);
	if (!pDriver->HGET(strCacheKey, propertyName, strValue))
	{
		return 0;
	}

	return lexical_cast<double>(strValue);
}

std::string CommonRedisModule::GetPropertyString(const std::string & self, const std::string & propertyName)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return "";
	}

	std::string strValue;
	std::string strCacheKey = GetPropertyCacheKey(self);
	if (!pDriver->HGET(strCacheKey, propertyName, strValue))
	{
		return "";
	}

	return strValue;
}

Guid CommonRedisModule::GetPropertyObject(const std::string & self, const std::string & propertyName)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return Guid();
	}

	std::string strValue;
	std::string strCacheKey = GetPropertyCacheKey(self);
	if (!pDriver->HGET(strCacheKey, propertyName, strValue))
	{
		return Guid();
	}
	Guid xID;
	xID.FromString(strValue);
	return xID;
}

Vector2 CommonRedisModule::GetPropertyVector2(const std::string & self, const std::string & propertyName)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return Vector2();
	}

	std::string strValue;
	std::string strCacheKey = GetPropertyCacheKey(self);
	if (!pDriver->HGET(strCacheKey, propertyName, strValue))
	{
		return Vector2();
	}

	Vector2 xValue;
	xValue.FromString(strValue);
	return xValue;
}

Vector3 CommonRedisModule::GetPropertyVector3(const std::string & self, const std::string & propertyName)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return Vector3();
	}

	std::string strValue;
	std::string strCacheKey = GetPropertyCacheKey(self);
	if (!pDriver->HGET(strCacheKey, propertyName, strValue))
	{
		return Vector3();
	}

	Vector3 xValue;
	xValue.FromString(strValue);
	return xValue;
}

bool CommonRedisModule::ConvertVectorToPropertyManager(std::vector<std::string>& vKeyList, std::vector<std::string>& vValueList, SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager, const bool cache, const bool save)
{
	if (vKeyList.size() == vValueList.size())
	{
		for (int i = 0; i < vKeyList.size(); ++i)
		{
			const std::string& strKey = vKeyList[i];
			const std::string& value = vValueList[i];
			if (value.empty())
			{
				continue;
			}

			//std::cout << strKey << ":" << strValue << std::endl;

			SQUICK_SHARE_PTR<IProperty> pProperty = pPropertyManager->GetElement(strKey);
			if ((cache && pProperty->GetCache()) || (save && pProperty->GetSave()))
			{
				if(!pProperty->FromString(value))
				{
					//TODO
					//error
				}
			}
		}
	}

    return true;
}

bool CommonRedisModule::ConvertVectorToRecordManager(std::vector<std::string>& vKeyList, std::vector<std::string>& vValueList, SQUICK_SHARE_PTR<IRecordManager> pRecordManager, const bool cache, const bool save)
{
	if (vKeyList.size() == vValueList.size())
	{
		for (int i = 0; i < vKeyList.size(); ++i)
		{
			const std::string& strKey = vKeyList[i];
			const std::string& value = vValueList[i];
			if (value.empty())
			{
				continue;
			}

			SQUICK_SHARE_PTR<IRecord> pRecord = pRecordManager->GetElement(strKey);
			if ((cache && pRecord->GetCache()) || (save && pRecord->GetSave()))
			{
				SquickStruct::ObjectRecordBase xRecordData;
				if (xRecordData.ParseFromString(value))
				{
					ConvertPBToRecord(xRecordData, pRecord);
				}
				else
				{
					//TODO
					//error
				}
			}
		}
	}

    return true;
}

bool CommonRedisModule::ConvertPropertyManagerToVector(SQUICK_SHARE_PTR<IPropertyManager> pPropertyManager, std::vector<std::string>& vKeyList, std::vector<std::string>& vValueList, const bool cache, const bool save)
{
	for (SQUICK_SHARE_PTR<IProperty> pProperty = pPropertyManager->First(); pProperty != NULL; pProperty = pPropertyManager->Next())
	{
		//const int type = pProperty->GetType();
		if ((cache && pProperty->GetCache()) || (save && pProperty->GetSave()))
		{
			const std::string& propertyName = pProperty->GetKey();
			const std::string& strPropertyValue = pProperty->ToString();

			//std::cout << propertyName << ":" << strPropertyValue << std::endl;

			vKeyList.push_back(propertyName);
			vValueList.push_back(strPropertyValue);
		}
	}

	return true;
}

bool CommonRedisModule::ConvertRecordManagerToVector(SQUICK_SHARE_PTR<IRecordManager> pRecordManager, std::vector<std::string>& vKeyList, std::vector<std::string>& vValueList, const bool cache, const bool save)
{
	for (SQUICK_SHARE_PTR<IRecord> pRecord = pRecordManager->First(); pRecord != NULL; pRecord = pRecordManager->Next())
	{
		if ((cache && pRecord->GetCache()) || (save && pRecord->GetSave()))
		{
			SquickStruct::ObjectRecordBase xRecordData;

			ConvertRecordToPB(pRecord, &xRecordData);

			////
			std::string strValue;
			if (!xRecordData.SerializeToString(&strValue))
			{
				continue;
			}

			vKeyList.push_back(xRecordData.record_name());
			vValueList.push_back(strValue);
		}
	}

	return true;
}

//bool CommonRedisModule::ConvertRecordToPB(const SQUICK_SHARE_PTR<IRecord> pRecord, SquickStruct::ObjectRecordBase * pRecordData)


//bool CommonRedisModule::ConvertPBToRecord(const SquickStruct::ObjectRecordBase& pRecordData, SQUICK_SHARE_PTR<IRecord> pRecord)

//bool CommonRedisModule::ConvertRecordManagerToPB(const SQUICK_SHARE_PTR<IRecordManager> pRecordManager, SquickStruct::ObjectRecordList * pRecordDataList, const bool cache, const bool save)


//bool CommonRedisModule::ConvertPBToRecordManager(const SquickStruct::ObjectRecordList & pRecordData, SQUICK_SHARE_PTR<IRecordManager> pRecord)

//bool CommonRedisModule::ConvertPropertyManagerToPB(const SQUICK_SHARE_PTR<IPropertyManager> pProps, SquickStruct::ObjectPropertyList * pPropertyData, const bool cache, const bool save)


//bool CommonRedisModule::ConvertPBToPropertyManager(const SquickStruct::ObjectPropertyList& pPropertyData, SQUICK_SHARE_PTR<IPropertyManager> pProps)


bool CommonRedisModule::SavePropertyInfo(const std::string &self, const std::string &propertyName, const std::string &propertyValue)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return false;
	}

	std::vector<std::string> vKeyList;
	std::vector<std::string> vValueList;

	vKeyList.push_back(propertyName);
	vValueList.push_back(propertyValue);

	if (vKeyList.size() != vValueList.size())
	{
		return false;
	}

	std::string strKey= GetPropertyCacheKey(self);

	if (!pDriver->HMSET(strKey, vKeyList, vValueList))
	{
		return false;
	}

	return false;
}

bool CommonRedisModule::GetPropertyList(const std::string &self, std::vector<std::pair<std::string, std::string>> &values)
{
	SQUICK_SHARE_PTR<IRedisClient> pDriver = m_pNoSqlModule->GetDriverBySuit(self);
	if (!pDriver)
	{
		return false;
	}

	std::string strCacheKey = GetPropertyCacheKey(self);
	if (pDriver->HGETALL(strCacheKey, values))
	{
		return true;
	}

	return false;
}
