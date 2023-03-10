

#include "object.h"
#include "record_manager.h"
#include "property_manager.h"

Object::Object(Guid self, IPluginManager* pLuginManager)
    : IObject(self)
{
	mObjectEventState = COE_CREATE_NODATA;

    mSelf = self;
    m_pPluginManager = pLuginManager;

    m_pRecordManager = SQUICK_SHARE_PTR<RecordManager>(SQUICK_NEW RecordManager(mSelf));
    m_pPropertyManager = SQUICK_SHARE_PTR<PropertyManager>(SQUICK_NEW PropertyManager(mSelf));
}

Object::~Object()
{

}

bool Object::AddRecordCallBack(const std::string& recordName, const RECORD_EVENT_FUNCTOR_PTR& cb)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        pRecord->AddRecordHook(cb);

        return true;
    }

    return false;
}

bool Object::AddPropertyCallBack(const std::string& strCriticalName, const PROPERTY_EVENT_FUNCTOR_PTR& cb)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(strCriticalName);
    if (pProperty)
    {
        pProperty->RegisterCallback(cb);

        return true;
    }

    return false;
}

CLASS_OBJECT_EVENT Object::GetState()
{
	return mObjectEventState;
}

void Object::SetState(const CLASS_OBJECT_EVENT eState)
{
	mObjectEventState = eState;
}

bool Object::ObjectReady()
{
    CLASS_OBJECT_EVENT state = GetState();
    if (state == COE_CREATE_HASDATA
        || state == COE_CREATE_FINISH
        || state == COE_CREATE_CLIENT_FINISH)
    {
        return true;
    }

    return false;
}

inline SQUICK_SHARE_PTR<IProperty> Object::AddProperty(const std::string& propertyName, const DATA_TYPE varType)
{
   return this->GetPropertyManager()->AddProperty(this->Self(), propertyName, varType);
}

bool Object::FindProperty(const std::string& propertyName)
{
    if (GetPropertyManager()->GetElement(propertyName))
    {
        return true;
    }

    return false;
}

bool Object::SetPropertyInt(const std::string& propertyName, const INT64 nValue, const INT64 reason)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
    if (pProperty)
    {
        return pProperty->SetInt(nValue, reason);
    }

    return false;
}

bool Object::SetPropertyFloat(const std::string& propertyName, const double dwValue, const INT64 reason)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
    if (pProperty)
    {
        return pProperty->SetFloat(dwValue, reason);
    }

    return false;
}

bool Object::SetPropertyString(const std::string& propertyName, const std::string& value, const INT64 reason)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
    if (pProperty)
    {
        return pProperty->SetString(value, reason);
    }

    return false;
}

bool Object::SetPropertyObject(const std::string& propertyName, const Guid& obj, const INT64 reason)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
    if (pProperty)
    {
        return pProperty->SetObject(obj, reason);
    }

    return false;
}

bool Object::SetPropertyVector2(const std::string& propertyName, const Vector2& value, const INT64 reason)
{
	SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
	if (pProperty)
	{
		return pProperty->SetVector2(value, reason);
	}

	return false;
}

bool Object::SetPropertyVector3(const std::string& propertyName, const Vector3& value, const INT64 reason)
{
	SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
	if (pProperty)
	{
		return pProperty->SetVector3(value, reason);
	}

	return false;
}

INT64 Object::GetPropertyInt(const std::string& propertyName)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
    if (pProperty)
    {
        return pProperty->GetInt();
    }

    return 0;
}

int Object::GetPropertyInt32(const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
	if (pProperty)
	{
		return pProperty->GetInt32();
	}

	return 0;
}

double Object::GetPropertyFloat(const std::string& propertyName)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
    if (pProperty)
    {
        return pProperty->GetFloat();
    }

    return 0.0;
}

const std::string& Object::GetPropertyString(const std::string& propertyName)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
    if (pProperty)
    {
        return pProperty->GetString();
    }

    return NULL_STR;
}

const Guid& Object::GetPropertyObject(const std::string& propertyName)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
    if (pProperty)
    {
        return pProperty->GetObject();
    }

    return NULL_OBJECT;
}

const Vector2& Object::GetPropertyVector2(const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
	if (pProperty)
	{
		return pProperty->GetVector2();
	}

	return NULL_VECTOR2;
}

const Vector3& Object::GetPropertyVector3(const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IProperty> pProperty = GetPropertyManager()->GetElement(propertyName);
	if (pProperty)
	{
		return pProperty->GetVector3();
	}

	return NULL_VECTOR3;
}

SQUICK_SHARE_PTR<IRecord> Object::FindRecord(const std::string& recordName)
{
	return GetRecordManager()->GetElement(recordName);
}

bool Object::SetRecordInt(const std::string& recordName, const int row, const int col, const INT64 nValue)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetInt(row, col, nValue);
    }

    return false;
}

bool Object::SetRecordInt(const std::string& recordName, const int row, const std::string& colTag, const INT64 value)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetInt(row, colTag, value);
    }

    return false;
}

bool Object::SetRecordFloat(const std::string& recordName, const int row, const int col, const double dwValue)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetFloat(row, col, dwValue);
    }

    return false;
}

bool Object::SetRecordFloat(const std::string& recordName, const int row, const std::string& colTag, const double value)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetFloat(row, colTag, value);
    }

    return false;
}

bool Object::SetRecordString(const std::string& recordName, const int row, const int col, const std::string& value)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetString(row, col, value.c_str());
    }

    return false;
}

bool Object::SetRecordString(const std::string& recordName, const int row, const std::string& colTag, const std::string& value)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetString(row, colTag, value.c_str());
    }

    return false;
}

bool Object::SetRecordObject(const std::string& recordName, const int row, const int col, const Guid& obj)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetObject(row, col, obj);
    }

    return false;
}

bool Object::SetRecordObject(const std::string& recordName, const int row, const std::string& colTag, const Guid& value)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetObject(row, colTag, value);
    }

    return false;
}

bool Object::SetRecordVector2(const std::string& recordName, const int row, const int col, const Vector2& obj)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
	if (pRecord)
	{
		return pRecord->SetVector2(row, col, obj);
	}

	return false;
}

bool Object::SetRecordVector2(const std::string& recordName, const int row, const std::string& colTag, const Vector2& value)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
	if (pRecord)
	{
		return pRecord->SetVector2(row, colTag, value);
	}

	return false;
}

bool Object::SetRecordVector3(const std::string& recordName, const int row, const int col, const Vector3& obj)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
	if (pRecord)
	{
		return pRecord->SetVector3(row, col, obj);
	}

	return false;
}

bool Object::SetRecordVector3(const std::string& recordName, const int row, const std::string& colTag, const Vector3& value)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
	if (pRecord)
	{
		return pRecord->SetVector3(row, colTag, value);
	}

	return false;
}

INT64 Object::GetRecordInt(const std::string& recordName, const int row, const int col)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetInt(row, col);
    }

    return 0;
}

INT64 Object::GetRecordInt(const std::string& recordName, const int row, const std::string& colTag)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetInt(row, colTag);
    }

    return 0;
}

double Object::GetRecordFloat(const std::string& recordName, const int row, const int col)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetFloat(row, col);
    }

    return 0.0;
}

double Object::GetRecordFloat(const std::string& recordName, const int row, const std::string& colTag)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetFloat(row, colTag);
    }

    return 0.0;
}

const std::string& Object::GetRecordString(const std::string& recordName, const int row, const int col)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetString(row, col);
    }

    return NULL_STR;
}

const std::string& Object::GetRecordString(const std::string& recordName, const int row, const std::string& colTag)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetString(row, colTag);
    }

    return NULL_STR;
}

const Guid& Object::GetRecordObject(const std::string& recordName, const int row, const int col)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetObject(row, col);
    }

    return NULL_OBJECT;
}

const Guid& Object::GetRecordObject(const std::string& recordName, const int row, const std::string& colTag)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetObject(row, colTag);
    }

    return NULL_OBJECT;
}

const Vector2& Object::GetRecordVector2(const std::string& recordName, const int row, const int col)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
	if (pRecord)
	{
		return pRecord->GetVector2(row, col);
	}

	return NULL_VECTOR2;
}

const Vector2& Object::GetRecordVector2(const std::string& recordName, const int row, const std::string& colTag)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
	if (pRecord)
	{
		return pRecord->GetVector2(row, colTag);
	}

	return NULL_VECTOR2;
}

const Vector3& Object::GetRecordVector3(const std::string& recordName, const int row, const int col)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
	if (pRecord)
	{
		return pRecord->GetVector3(row, col);
	}

	return NULL_VECTOR3;
}

const Vector3& Object::GetRecordVector3(const std::string& recordName, const int row, const std::string& colTag)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetRecordManager()->GetElement(recordName);
	if (pRecord)
	{
		return pRecord->GetVector3(row, colTag);
	}

	return NULL_VECTOR3;
}

SQUICK_SHARE_PTR<IRecordManager> Object::GetRecordManager()
{
    return m_pRecordManager;
}

SQUICK_SHARE_PTR<IPropertyManager> Object::GetPropertyManager()
{
    return m_pPropertyManager;
}

void Object::SetRecordManager(SQUICK_SHARE_PTR<IRecordManager> xRecordManager)
{
	m_pRecordManager = xRecordManager;
}

void Object::SetPropertyManager(SQUICK_SHARE_PTR<IPropertyManager> xPropertyManager)
{
	m_pPropertyManager = xPropertyManager;
}

Guid Object::Self()
{
    return mSelf;
}

void Object::ToMemoryCounterString(std::string& info)
{
    info.append(mSelf.ToString());
    info.append(":");
    info.append(m_pPropertyManager->GetPropertyString("ClassName"));
}

const Vector3 &Object::GetPosition()
{
	return mPosition;
}

void Object::SetPosition(const Vector3 &pos)
{
	mPosition = pos;
}
