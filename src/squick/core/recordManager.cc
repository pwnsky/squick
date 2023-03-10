
#include "record_manager.h"

RecordManager::~RecordManager()
{
    ClearAll();
}

SQUICK_SHARE_PTR<IRecord> RecordManager::AddRecord(const Guid& self, const std::string& recordName, const SQUICK_SHARE_PTR<DataList>& ValueList, const SQUICK_SHARE_PTR<DataList>& tagList, const int nRows)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (!pRecord)
    {
        //SQUICK_SHARE_PTR<IRecord>
        pRecord = SQUICK_SHARE_PTR<IRecord>(SQUICK_NEW Record(self, recordName, ValueList, tagList, nRows));
        this->AddElement(recordName, pRecord);
    }

    return pRecord;
}

const Guid& RecordManager::Self()
{
    return mSelf;
}

std::string RecordManager::ToString()
{
	std::stringstream stream;
	SQUICK_SHARE_PTR<IRecord> pRecord = First();
	while (pRecord)
	{
		stream << pRecord->ToString() << std::endl;
		pRecord = Next();
	}

	return stream.str();
}

bool RecordManager::SetRecordInt(const std::string& recordName, const int row, const int col, const INT64 nValue)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetInt(row, col, nValue);
    }

    return false;
}

bool RecordManager::SetRecordInt(const std::string& recordName, const int row, const std::string& colTag, const INT64 value)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetInt(row, colTag, value);
    }

    return false;
}

bool RecordManager::SetRecordFloat(const std::string& recordName, const int row, const int col, const double dwValue)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetFloat(row, col, dwValue);
    }

    return false;
}

bool RecordManager::SetRecordFloat(const std::string& recordName, const int row, const std::string& colTag, const double value)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetFloat(row, colTag, value);
    }

    return false;
}

bool RecordManager::SetRecordString(const std::string& recordName, const int row, const int col, const std::string& value)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetString(row, col, value);
    }

    return false;
}

bool RecordManager::SetRecordString(const std::string& recordName, const int row, const std::string& colTag, const std::string& value)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetString(row, colTag, value);
    }

    return false;
}

bool RecordManager::SetRecordObject(const std::string& recordName, const int row, const int col, const Guid& obj)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetObject(row, col, obj);
    }

    return false;
}

bool RecordManager::SetRecordObject(const std::string& recordName, const int row, const std::string& colTag, const Guid& value)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->SetObject(row, colTag, value);
    }

    return false;
}

bool RecordManager::SetRecordVector2(const std::string& recordName, const int row, const int col, const Vector2& obj)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
	if (pRecord)
	{
		return pRecord->SetVector2(row, col, obj);
	}

	return false;
}

bool RecordManager::SetRecordVector2(const std::string& recordName, const int row, const std::string& colTag, const Vector2& value)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
	if (pRecord)
	{
		return pRecord->SetVector2(row, colTag, value);
	}

	return false;
}

bool RecordManager::SetRecordVector3(const std::string& recordName, const int row, const int col, const Vector3& obj)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
	if (pRecord)
	{
		return pRecord->SetVector3(row, col, obj);
	}

	return false;
}

bool RecordManager::SetRecordVector3(const std::string& recordName, const int row, const std::string& colTag, const Vector3& value)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
	if (pRecord)
	{
		return pRecord->SetVector3(row, colTag, value);
	}

	return false;
}

INT64 RecordManager::GetRecordInt(const std::string& recordName, const int row, const int col)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetInt(row, col);
    }

    return 0;
}

INT64 RecordManager::GetRecordInt(const std::string& recordName, const int row, const std::string& colTag)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetInt(row, colTag);
    }

    return 0;
}


double RecordManager::GetRecordFloat(const std::string& recordName, const int row, const int col)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetFloat(row, col);
    }

    return 0.0;
}

double RecordManager::GetRecordFloat(const std::string& recordName, const int row, const std::string& colTag)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetFloat(row, colTag);
    }

    return 0.0;
}

const std::string& RecordManager::GetRecordString(const std::string& recordName, const int row, const int col)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetString(row, col);
    }

    return NULL_STR;
}

const std::string& RecordManager::GetRecordString(const std::string& recordName, const int row, const std::string& colTag)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetString(row, colTag);
    }

    return NULL_STR;
}

const Guid& RecordManager::GetRecordObject(const std::string& recordName, const int row, const int col)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetObject(row, col);
    }

    return NULL_OBJECT;
}

const Guid& RecordManager::GetRecordObject(const std::string& recordName, const int row, const std::string& colTag)
{
    SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
    if (pRecord)
    {
        return pRecord->GetObject(row, colTag);
    }

    return NULL_OBJECT;
}

const Vector2& RecordManager::GetRecordVector2(const std::string& recordName, const int row, const int col)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
	if (pRecord)
	{
		return pRecord->GetVector2(row, col);
	}

	return NULL_VECTOR2;
}

const Vector2& RecordManager::GetRecordVector2(const std::string& recordName, const int row, const std::string& colTag)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
	if (pRecord)
	{
		return pRecord->GetVector2(row, colTag);
	}

	return NULL_VECTOR2;
}

const Vector3& RecordManager::GetRecordVector3(const std::string& recordName, const int row, const int col)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
	if (pRecord)
	{
		return pRecord->GetVector3(row, col);
	}

	return NULL_VECTOR3;
}

const Vector3& RecordManager::GetRecordVector3(const std::string& recordName, const int row, const std::string& colTag)
{
	SQUICK_SHARE_PTR<IRecord> pRecord = GetElement(recordName);
	if (pRecord)
	{
		return pRecord->GetVector3(row, colTag);
	}

	return NULL_VECTOR3;
}
