
#include <exception>
#include "data_list.h"
#include "record.h"

Record::Record()
{
    mSelf = Guid();

    mbSave = false;
    mbPublic = false;
    mbPrivate = false;
    mbCache = false;
	mbRef = false;
	mbForce = false;
	mbUpload = false;

    mstrRecordName = "";
    mnMaxRow = 0;

}

Record::Record(const Guid& self, const std::string& recordName, const SQUICK_SHARE_PTR<DataList>& valueList, const SQUICK_SHARE_PTR<DataList>& tagList, const int nMaxRow)
{
	mVarRecordType = valueList;
    mVarRecordTag = tagList;

    mSelf = self;

    mbSave = false;
    mbPublic = false;
    mbPrivate = false;
    mbCache = false;
	mbUpload = false;

    mstrRecordName = recordName;

    mnMaxRow = nMaxRow;
    
    mVecUsedState.resize(mnMaxRow);

    for (int i = 0; i < mnMaxRow; i++)
    {
        mVecUsedState[i] = 0;
    }

	//init share_pointer for all data
    for (int i = 0; i < GetRows() * GetCols(); i++)
    {
        mtRecordVec.push_back(SQUICK_SHARE_PTR<SquickData>());
    }

	//it would be optimized in future as it should apply the memory by onetime
    for (int i = 0; i < mVarRecordTag->GetCount(); ++i)
    {
        if (!mVarRecordTag->String(i).empty())
        {
            mmTag[mVarRecordTag->String(i)] = i;
        }
    }
}

Record::~Record()
{
    for (TRECORDVEC::iterator iter = mtRecordVec.begin(); iter != mtRecordVec.end(); ++iter)
    {
        iter->reset();
    }

    for (TRECORDCALLBACKEX::iterator iter = mtRecordCallback.begin(); iter != mtRecordCallback.end(); ++iter)
    {
        iter->reset();
    }

    mtRecordVec.clear();
    mVecUsedState.clear();
    mtRecordCallback.clear();
}

std::string Record::ToString()
{
    std::stringstream ss;
    ss << this->GetName() << std::endl;

    for (int i = 0; i < this->GetRows(); ++i)
    {
        if (IsUsed(i))
        {
            DataList rowDataList;
            if (this->QueryRow(i, rowDataList))
            {
                ss << "ROW:" << i << "==>" << rowDataList.ToString() << std::endl;
            }
        }
    }

    return ss.str();
}
void Record::ToMemoryCounterString(std::string& data)
{
    data.append(this->mSelf.ToString());
    data.append(":");
    data.append(this->GetName());
    data.append(" ");
}

int Record::GetCols() const
{
    return mVarRecordType->GetCount();
}

int Record::GetRows() const
{
    return mnMaxRow;
}

int Record::GetUsedRows() const
{
	int usedCount = 0;
	for (int i = 0; i < mVecUsedState.size(); ++i)
	{
		if (mVecUsedState[i] > 0)
		{
			usedCount++;
		}
	}

	return usedCount;
}

DATA_TYPE Record::GetColType(const int col) const
{
    return mVarRecordType->Type(col);
}

const std::string& Record::GetColTag(const int col) const
{
    return mVarRecordTag->String(col);
}


int Record::AddRow(const int row)
{
    return AddRow(row, *mVarRecordType);
}

int Record::AddRow(const int row, const DataList& var)
{
	bool bCover = false;
    int nFindRow = row;
    if (nFindRow >= mnMaxRow)
    {
        return -1;
    }

    if (var.GetCount() != GetCols())
    {
        return -1;
    }

    if (nFindRow < 0)
    {
        for (int i = 0; i < mnMaxRow; i++)
        {
            if (!IsUsed(i))
            {
                nFindRow = i;
                break;
            }
        }
    }
	else
	{
		if (IsUsed(nFindRow))
	    {
	        bCover = true;
	    }		
	}

    if (nFindRow < 0)
    {
        return -1;
    }

    for (int i = 0; i < GetCols(); ++i)
    {
        if (var.Type(i) != GetColType(i))
        {
            return -1;
        }
    }

    SetUsed(nFindRow, 1);

    for (int i = 0; i < GetCols(); ++i)
    {
        SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(nFindRow, i));
		if (nullptr == pVar)
		{
			pVar = SQUICK_SHARE_PTR<SquickData>(SQUICK_NEW SquickData(var.Type(i)));
		}

		pVar->variantData = var.GetStack(i)->variantData;
    }

	RECORD_EVENT_DATA eventData;
	eventData.nOpType = bCover? RECORD_EVENT_DATA::Cover : RECORD_EVENT_DATA::Add;
	eventData.row = nFindRow;
	eventData.col = 0;
	eventData.recordName = mstrRecordName;
	eventData.recordData = this;

	SquickData tData;
    OnEventHandler(mSelf, eventData, tData, tData);

    return nFindRow;
}

bool Record::SetRow(const int row, const DataList & var)
{
	if (var.GetCount() != GetCols())
	{
		return false;
	}

	if (!IsUsed(row))
	{
		return false;
	}

	for (int i = 0; i < GetCols(); ++i)
	{
		if (var.Type(i) != GetColType(i))
		{
			return false;
		}
	}

	for (int i = 0; i < GetCols(); ++i)
	{
		SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, i));
		if (nullptr == pVar)
		{
			pVar = SQUICK_SHARE_PTR<SquickData>(SQUICK_NEW SquickData(var.Type(i)));
		}

		SquickData oldValue = *pVar;

		pVar->variantData = var.GetStack(i)->variantData;

		RECORD_EVENT_DATA eventData;
		eventData.nOpType = RECORD_EVENT_DATA::Update;
		eventData.row = row;
		eventData.col = i;
		eventData.recordName = mstrRecordName;
		eventData.recordData = this;

		OnEventHandler(mSelf, eventData, oldValue, *pVar);
	}

	return false;
}

bool Record::SetInt(const int row, const int col, const INT64 value)
{
    if (!ValidPos(row, col))
    {
        return false;
    }

    if (TDATA_INT != GetColType(col))
    {
        return false;
    }

    if (!IsUsed(row))
    {
        return false;
    }

	SquickData var;
    var.SetInt(value);

    SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));
	//must have memory
	if (nullptr == pVar)
	{
		return false;
	}

	if (var == *pVar)
	{
		return true;
	}

	if (mtRecordCallback.size() == 0)
	{
		pVar->SetInt(value);
	}
	else
	{
		SquickData oldValue;
		oldValue.SetInt(pVar->GetInt());

		pVar->SetInt(value);

		RECORD_EVENT_DATA eventData;
		eventData.nOpType = RECORD_EVENT_DATA::Update;
		eventData.row = row;
		eventData.col = col;
		eventData.recordName = mstrRecordName;
		eventData.recordData = this;

		OnEventHandler(mSelf, eventData, oldValue, *pVar);
	}

    return true;
}

bool Record::SetInt(const int row, const std::string& colTag, const INT64 value)
{
    int col = GetCol(colTag);
    return SetInt(row, col, value);
}

bool Record::SetFloat(const int row, const int col, const double value)
{
    if (!ValidPos(row, col))
    {
        return false;
    }

    if (TDATA_FLOAT != GetColType(col))
    {
        return false;
    }

    if (!IsUsed(row))
    {
        return false;
    }

    SquickData var;
    var.SetFloat(value);

    SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));

	//must have memory
	if (nullptr == pVar)
	{
		return false;
	}

	if (var == *pVar)
	{
		return true;
	}

	if (mtRecordCallback.size() == 0)
	{
		pVar->SetFloat(value);
	}
	else
	{
		SquickData oldValue;
		oldValue.SetFloat(pVar->GetFloat());
		pVar->SetFloat(value);

		RECORD_EVENT_DATA eventData;
		eventData.nOpType = RECORD_EVENT_DATA::Update;
		eventData.row = row;
		eventData.col = col;
		eventData.recordName = mstrRecordName;
		eventData.recordData = this;

		OnEventHandler(mSelf, eventData, oldValue, *pVar);
	}

    return true;
}

bool Record::SetFloat(const int row, const std::string& colTag, const double value)
{
    int col = GetCol(colTag);
    return SetFloat(row, col, value);
}

bool Record::SetString(const int row, const int col, const std::string& value)
{
    if (!ValidPos(row, col))
    {
        return false;
    }

    if (TDATA_STRING != GetColType(col))
    {
        return false;
    }

    if (!IsUsed(row))
    {
        return false;
    }

    SquickData var;
    var.SetString(value);

    SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));

	//must have memory
	if (nullptr == pVar)
	{
		return false;
	}

	if (var == *pVar)
	{
		return true;
	}

	if (mtRecordCallback.size() == 0)
	{
		pVar->SetString(value);
	}
	else
	{
		SquickData oldValue;
		oldValue.SetString(pVar->GetString());

		pVar->SetString(value);

		RECORD_EVENT_DATA eventData;
		eventData.nOpType = RECORD_EVENT_DATA::Update;
		eventData.row = row;
		eventData.col = col;
		eventData.recordName = mstrRecordName;
		eventData.recordData = this;

		OnEventHandler(mSelf, eventData, oldValue, *pVar);
	}

    return true;
}

bool Record::SetString(const int row, const std::string& colTag, const std::string& value)
{
    int col = GetCol(colTag);
    return SetString(row, col, value);
}

bool Record::SetObject(const int row, const int col, const Guid& value)
{
    if (!ValidPos(row, col))
    {
        return false;
    }

    if (TDATA_OBJECT != GetColType(col))
    {
        return false;
    }

    if (!IsUsed(row))
    {
        return false;
    }

    SquickData var;
    var.SetObject(value);

    SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));

	//must have memory
	if (nullptr == pVar)
	{
		return false;
	}

	if (var == *pVar)
	{
		return true;
	}

	if (mtRecordCallback.size() == 0)
	{
		pVar->SetObject(value);
	}
	else
	{
		SquickData oldValue;
		oldValue.SetObject(pVar->GetObject());

		pVar->SetObject(value);

		RECORD_EVENT_DATA eventData;
		eventData.nOpType = RECORD_EVENT_DATA::Update;
		eventData.row = row;
		eventData.col = col;
		eventData.recordName = mstrRecordName;
		eventData.recordData = this;

		OnEventHandler(mSelf, eventData, oldValue, *pVar);
	}

    return true;
}

bool Record::SetObject(const int row, const std::string& colTag, const Guid& value)
{
	int col = GetCol(colTag);
	return SetObject(row, col, value);
}

bool Record::SetVector2(const int row, const int col, const Vector2& value)
{
	if (!ValidPos(row, col))
	{
		return false;
	}

	if (TDATA_VECTOR2 != GetColType(col))
	{
		return false;
	}

	if (!IsUsed(row))
	{
		return false;
	}

	SquickData var;
	var.SetVector2(value);

	SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));

	//must have memory
	if (nullptr == pVar)
	{
		return false;
	}

	if (var == *pVar)
	{
		return true;
	}

	if (mtRecordCallback.size() == 0)
	{
		pVar->SetVector2(value);
	}
	else
	{
		SquickData oldValue;
		oldValue.SetVector2(pVar->GetVector2());

		pVar->SetVector2(value);

		RECORD_EVENT_DATA eventData;
		eventData.nOpType = RECORD_EVENT_DATA::Update;
		eventData.row = row;
		eventData.col = col;
		eventData.recordName = mstrRecordName;

		OnEventHandler(mSelf, eventData, oldValue, *pVar);
	}

	return true;
}

bool Record::SetVector3(const int row, const int col, const Vector3& value)
{
	if (!ValidPos(row, col))
	{
		return false;
	}

	if (TDATA_VECTOR3 != GetColType(col))
	{
		return false;
	}

	if (!IsUsed(row))
	{
		return false;
	}

	SquickData var;
	var.SetVector3(value);

	SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));

	//must have memory
	if (nullptr == pVar)
	{
		return false;
	}

	if (var == *pVar)
	{
		return true;
	}

	if (mtRecordCallback.size() == 0)
	{
		pVar->SetVector3(value);
	}
	else
	{
		SquickData oldValue;
		oldValue.SetVector3(pVar->GetVector3());

		pVar->SetVector3(value);

		RECORD_EVENT_DATA eventData;
		eventData.nOpType = RECORD_EVENT_DATA::Update;
		eventData.row = row;
		eventData.col = col;
		eventData.recordName = mstrRecordName;

		OnEventHandler(mSelf, eventData, oldValue, *pVar);
	}

	return true;
}

bool Record::SetVector2(const int row, const std::string& colTag, const Vector2& value)
{
	int col = GetCol(colTag);
	return SetVector2(row, col, value);
}

bool Record::SetVector3(const int row, const std::string& colTag, const Vector3& value)
{
	int col = GetCol(colTag);
	return SetVector3(row, col, value);
}


bool Record::QueryRow(const int row, DataList& varList)
{
    if (!ValidRow(row))
    {
        return false;
    }

    if (!IsUsed(row))
    {
        return false;
    }

    varList.Clear();
    for (int i = 0; i < GetCols(); ++i)
    {
        SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, i));
        if (pVar)
        {
            varList.Append(*pVar);
        }
        else
        {
            switch (GetColType(i))
            {
                case TDATA_INT:
                    varList.Add(INT64(0));
                    break;

                case TDATA_FLOAT:
                    varList.Add(0.0f);
                    break;

                case TDATA_STRING:
                    varList.Add(NULL_STR.c_str());
                    break;

                case TDATA_OBJECT:
                    varList.Add(Guid());
                    break;

				case TDATA_VECTOR2:
					varList.Add(Vector2());
					break;

				case TDATA_VECTOR3:
					varList.Add(Vector3());
					break;
                default:
                    return false;
                    break;
            }
        }
    }

    if (varList.GetCount() != GetCols())
    {
        return false;
    }

    return true;
}

INT64 Record::GetInt(const int row, const int col) const
{
    if (!ValidPos(row, col))
    {
        return 0;
    }

    if (!IsUsed(row))
    {
        return 0;
    }

    const SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));
    if (!pVar)
    {
        return 0;
    }

    return pVar->GetInt();
}

INT64 Record::GetInt(const int row, const std::string& colTag) const
{
    int col = GetCol(colTag);
    return GetInt(row, col);
}

double Record::GetFloat(const int row, const int col) const
{
    if (!ValidPos(row, col))
    {
        return 0.0f;
    }

    if (!IsUsed(row))
    {
        return 0.0f;
    }

    const SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));
    if (!pVar)
    {
        return 0.0f;
    }

    return pVar->GetFloat();
}

double Record::GetFloat(const int row, const std::string& colTag) const
{
    int col = GetCol(colTag);
    return GetFloat(row, col);
}

const std::string& Record::GetString(const int row, const int col) const
{
    if (!ValidPos(row, col))
    {
        return NULL_STR;
    }

    if (!IsUsed(row))
    {
        return NULL_STR;
    }

    const SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));
    if (!pVar)
    {
        return NULL_STR;
    }

    return pVar->GetString();
}

const std::string& Record::GetString(const int row, const std::string& colTag) const
{
    int col = GetCol(colTag);
    return GetString(row, col);
}

const Guid& Record::GetObject(const int row, const int col) const
{
    if (!ValidPos(row, col))
    {
        return NULL_OBJECT;
    }

    if (!IsUsed(row))
    {
        return NULL_OBJECT;
    }

    const  SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));
    if (!pVar)
    {
        return NULL_OBJECT;
    }

    return pVar->GetObject();
}

const Guid& Record::GetObject(const int row, const std::string& colTag) const
{
    int col = GetCol(colTag);
    return GetObject(row, col);
}

const Vector2& Record::GetVector2(const int row, const int col) const
{
	if (!ValidPos(row, col))
	{
		return NULL_VECTOR2;
	}

	if (!IsUsed(row))
	{
		return NULL_VECTOR2;
	}

	const  SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));
	if (!pVar)
	{
		return NULL_VECTOR2;
	}

	return pVar->GetVector2();
}

const Vector2& Record::GetVector2(const int row, const std::string& colTag) const
{
	int col = GetCol(colTag);
	return GetVector2(row, col);
}

const Vector3& Record::GetVector3(const int row, const int col) const
{
	if (!ValidPos(row, col))
	{
		return NULL_VECTOR3;
	}

	if (!IsUsed(row))
	{
		return NULL_VECTOR3;
	}

	const  SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, col));
	if (!pVar)
	{
		return NULL_VECTOR3;
	}

	return pVar->GetVector3();
}

const Vector3& Record::GetVector3(const int row, const std::string& colTag) const
{
	int col = GetCol(colTag);
	return GetVector3(row, col);
}

int Record::FindRowByColValue(const int col, const SquickData& var, DataList& varResult)
{
    if (!ValidCol(col))
    {
        return -1;
    }

    DATA_TYPE eType = var.GetType();
    if (eType != mVarRecordType->Type(col))
    {
        return -1;
    }

    switch (eType)
    {
        case TDATA_INT:
            return FindInt(col, var.GetInt(), varResult);
            break;

        case TDATA_FLOAT:
            return FindFloat(col, var.GetFloat(), varResult);
            break;

        case TDATA_STRING:
            return FindString(col, var.GetString(), varResult);
            break;

        case TDATA_OBJECT:
            return FindObject(col, var.GetObject(), varResult);
            break;

		case TDATA_VECTOR2:
			return FindVector2At(col, var.GetVector2(), varResult);
			break;

        case TDATA_VECTOR3:
			return FindVector3At(col, var.GetVector3(), varResult);
			break;

        default:
            break;
    }

    return -1;
}

int Record::FindRowByColValue(const std::string& colTag, const SquickData& var, DataList& varResult)
{
    int col = GetCol(colTag);
    return FindRowByColValue(col, var, varResult);
}

int Record::FindInt(const int col, const INT64 value, DataList& varResult)
{
    if (!ValidCol(col))
    {
        return -1;
    }

    if (TDATA_INT != mVarRecordType->Type(col))
    {
        return -1;
    }

    {
        for (int i = 0; i < mnMaxRow; ++i)
        {
            if (!IsUsed(i))
            {
                continue;
            }

            if (GetInt(i, col) == value)
            {
                varResult << i;
            }
        }

        return varResult.GetCount();
    }

    return -1;
}

int Record::FindInt(const std::string& colTag, const INT64 value, DataList& varResult)
{
    if (colTag.empty())
    {
        return -1;
    }

    int col = GetCol(colTag);
    return FindInt(col, value, varResult);
}

int Record::FindFloat(const int col, const double value, DataList& varResult)
{
    if (!ValidCol(col))
    {
        return -1;
    }

    if (TDATA_FLOAT != mVarRecordType->Type(col))
    {
        return -1;
    }

    for (int i = 0; i < mnMaxRow; ++i)
    {
        if (!IsUsed(i))
        {
            continue;
        }

        if (GetFloat(i, col) == value)
        {
            varResult << i;
        }
    }

    return varResult.GetCount();
}

int Record::FindFloat(const std::string& colTag, const double value, DataList& varResult)
{
    if (colTag.empty())
    {
        return -1;
    }

    int col = GetCol(colTag);
    return FindFloat(col, value, varResult);
}

int Record::FindString(const int col, const std::string& value, DataList& varResult)
{
    if (!ValidCol(col))
    {
        return -1;
    }

    if (TDATA_STRING != mVarRecordType->Type(col))
    {
        return -1;
    }


    {
        for (int i = 0; i < mnMaxRow; ++i)
        {
            if (!IsUsed(i))
            {
                continue;
            }

            const std::string& strData = GetString(i, col);
            if (0 == strcmp(strData.c_str(), value.c_str()))
            {
                varResult << (int64_t)i;
            }
        }

        return varResult.GetCount();
    }

    return -1;
}

int Record::FindString(const std::string& colTag, const std::string& value, DataList& varResult)
{
    if (colTag.empty())
    {
        return -1;
    }

    int col = GetCol(colTag);
    return FindString(col, value, varResult);
}

int Record::FindObject(const int col, const Guid& value, DataList& varResult)
{
    if (!ValidCol(col))
    {
        return -1;
    }

    if (TDATA_OBJECT != mVarRecordType->Type(col))
    {
        return -1;
    }

    {
        for (int i = 0; i < mnMaxRow; ++i)
        {
            if (!IsUsed(i))
            {
                continue;
            }

            if (GetObject(i, col) == value)
            {
                varResult << (int64_t)i;
            }
        }

        return varResult.GetCount();
    }

    return -1;
}

int Record::FindObject(const std::string& colTag, const Guid& value, DataList& varResult)
{
    if (colTag.empty())
    {
        return -1;
    }

    int col = GetCol(colTag);
    return FindObject(col, value, varResult);
}

int Record::FindVector2At(const int col, const Vector2& value, DataList& varResult)
{
	if (!ValidCol(col))
	{
		return -1;
	}

	if (TDATA_VECTOR2 != mVarRecordType->Type(col))
	{
		return -1;
	}

	{
		for (int i = 0; i < mnMaxRow; ++i)
		{
			if (!IsUsed(i))
			{
				continue;
			}

			if (GetVector2(i, col) == value)
			{
				varResult << (int64_t)i;
			}
		}

		return varResult.GetCount();
	}

	return -1;
}

int Record::FindVector2At(const std::string& colTag, const Vector2& value, DataList& varResult)
{
	if (colTag.empty())
	{
		return -1;
	}

	int col = GetCol(colTag);
	return FindVector2At(col, value, varResult);
}

int Record::FindVector3At(const int col, const Vector3& value, DataList& varResult)
{
	if (!ValidCol(col))
	{
		return -1;
	}

	if (TDATA_VECTOR3 != mVarRecordType->Type(col))
	{
		return -1;
	}

	{
		for (int i = 0; i < mnMaxRow; ++i)
		{
			if (!IsUsed(i))
			{
				continue;
			}

			if (GetVector3(i, col) == value)
			{
				varResult << (int64_t)i;
			}
		}

		return varResult.GetCount();
	}

	return -1;
}

int Record::FindRowByColValue(const int col, const SquickData & var)
{
	DataList xDataList;
	int nRowCount = FindRowByColValue(col, var, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindInt(const int col, const INT64 value)
{
	DataList xDataList;
	int nRowCount = FindInt(col, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindFloat(const int col, const double value)
{
	DataList xDataList;
	int nRowCount = FindFloat(col, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindString(const int col, const std::string & value)
{
	DataList xDataList;
	int nRowCount = FindString(col, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindObject(const int col, const Guid & value)
{
	DataList xDataList;
	int nRowCount = FindObject(col, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindVector2At(const int col, const Vector2 & value)
{
	DataList xDataList;
	int nRowCount = FindVector2At(col, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindVector3At(const int col, const Vector3 & value)
{
	DataList xDataList;
	int nRowCount = FindVector3At(col, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindVector3At(const std::string& colTag, const Vector3& value, DataList& varResult)
{
	if (colTag.empty())
	{
		return -1;
	}

	int col = GetCol(colTag);
	return FindVector3At(col, value, varResult);
}

int Record::FindRowByColValue(const std::string & colTag, const SquickData & var)
{
	DataList xDataList;
	int nRowCount = FindRowByColValue(colTag, var, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindInt(const std::string & colTag, const INT64 value)
{
	DataList xDataList;
	int nRowCount = FindInt(colTag, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindFloat(const std::string & colTag, const double value)
{
	DataList xDataList;
	int nRowCount = FindFloat(colTag, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindString(const std::string & colTag, const std::string & value)
{
	DataList xDataList;
	int nRowCount = FindString(colTag, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindObject(const std::string & colTag, const Guid & value)
{
	DataList xDataList;
	int nRowCount = FindObject(colTag, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindVector2At(const std::string & colTag, const Vector2 & value)
{
	DataList xDataList;
	int nRowCount = FindVector2At(colTag, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

int Record::FindVector3At(const std::string & colTag, const Vector3 & value)
{
	DataList xDataList;
	int nRowCount = FindVector3At(colTag, value, xDataList);
	if (nRowCount > 0 && xDataList.GetCount() > 0)
	{
		return (int) xDataList.Int(0);
	}

	return -1;
}

bool Record::Remove(const int row)
{
    if (ValidRow(row))
    {
        if (IsUsed(row))
        {
			RECORD_EVENT_DATA eventData;
			eventData.nOpType = RECORD_EVENT_DATA::Del;
			eventData.row = row;
			eventData.col = 0;
			eventData.recordName = mstrRecordName;

			OnEventHandler(mSelf, eventData, SquickData(), SquickData());

			mVecUsedState[row] = 0;

			eventData.nOpType = RECORD_EVENT_DATA::AfterDel;

			OnEventHandler(mSelf, eventData, SquickData(), SquickData());

			return true;
        }
    }

    return false;
}

bool Record::Clear()
{
    for (int i = GetRows() - 1; i >= 0; i--)
    {
        Remove(i);
    }

    return true;
}

void Record::AddRecordHook(const RECORD_EVENT_FUNCTOR_PTR& cb)
{
    mtRecordCallback.push_back(cb);
}

const bool Record::GetSave()
{
    return mbSave;
}

const bool Record::GetCache()
{
    return mbCache;
}

const bool Record::GetRef()
{
	return mbRef;
}

const bool Record::GetForce()
{
	return mbForce;
}

const bool Record::GetUpload()
{
	return mbUpload;
}

const bool Record::GetPublic()
{
    return mbPublic;
}

const bool Record::GetPrivate()
{
    return mbPrivate;
}

int Record::GetPos(int row, int col) const
{
    return row * mVarRecordType->GetCount() + col;
}

const std::string& Record::GetName() const
{
    return mstrRecordName;
}

void Record::SetSave(const bool bSave)
{
    mbSave = bSave;
}

void Record::SetCache(const bool bCache)
{
    mbCache = bCache;
}

void Record::SetRef(const bool bRef)
{
	mbRef = bRef;
}

void Record::SetForce(const bool bForce)
{
	mbForce = bForce;
}

void Record::SetUpload(const bool bUpload)
{
	mbUpload = bUpload;
}

void Record::SetPublic(const bool bPublic)
{
    mbPublic = bPublic;
}

void Record::SetPrivate(const bool bPrivate)
{
    mbPrivate = bPrivate;
}

void Record::SetName(const std::string& name)
{
    mstrRecordName = name;
}

SQUICK_SHARE_PTR<DataList> Record::GetStartData() const
{
    SQUICK_SHARE_PTR<DataList> pIniData = SQUICK_SHARE_PTR<DataList>( SQUICK_NEW DataList());
    pIniData->Append(*mVarRecordType);

    return pIniData;
}

void Record::OnEventHandler(const Guid& self, const RECORD_EVENT_DATA& eventData, const SquickData& oldVar, const SquickData& newVar)
{
    TRECORDCALLBACKEX::iterator itr = mtRecordCallback.begin();
    TRECORDCALLBACKEX::iterator end = mtRecordCallback.end();
    for (; itr != end; ++itr)
    {
        
        RECORD_EVENT_FUNCTOR_PTR functorPtr = *itr;
        functorPtr->operator()(self, eventData, oldVar, newVar);
    }
}

bool Record::IsUsed(const int row) const
{
    if (ValidRow(row))
    {
        return (mVecUsedState[row] > 0);
    }

    return false;
}

bool Record::SwapRowInfo(const int nOriginRow, const int nTargetRow)
{
    if (!IsUsed(nOriginRow))
    {
        return false;
    }

    if (ValidRow(nOriginRow)
        && ValidRow(nTargetRow))
    {
        for (int i = 0; i < GetCols(); ++i)
        {
            SQUICK_SHARE_PTR<SquickData> pOrigin = mtRecordVec.at(GetPos(nOriginRow, i));
            mtRecordVec[GetPos(nOriginRow, i)] = mtRecordVec.at(GetPos(nTargetRow, i));
            mtRecordVec[GetPos(nTargetRow, i)] = pOrigin;
        }

        int nOriginUse = mVecUsedState[nOriginRow];
        mVecUsedState[nOriginRow] = mVecUsedState[nTargetRow];
        mVecUsedState[nTargetRow] = nOriginUse;

        RECORD_EVENT_DATA eventData;
        eventData.nOpType = RECORD_EVENT_DATA::Swap;
        eventData.row = nOriginRow;
        eventData.col = nTargetRow;
        eventData.recordName = mstrRecordName;

        SquickData xData;
        OnEventHandler(mSelf, eventData, xData, xData);

        return true;
    }

    return false;
}

const SQUICK_SHARE_PTR<DataList> Record::GetTag() const
{
	//TODO
    SQUICK_SHARE_PTR<DataList> pIniData = SQUICK_SHARE_PTR<DataList>(SQUICK_NEW DataList());
    pIniData->Append(*mVarRecordTag);
    return pIniData;
}

const IRecord::TRECORDVEC& Record::GetRecordVec() const
{
    return mtRecordVec;
}

bool Record::SetUsed(const int row, const int bUse)
{
    if (ValidRow(row))
    {
        mVecUsedState[row] = bUse;
        return true;
    }

    return false;
}

bool Record::PreAllocMemoryForRow(const int row)
{
	if (!IsUsed(row))
	{
		return false;
	}

	for (int i = 0; i < GetCols(); ++i)
	{
		SQUICK_SHARE_PTR<SquickData>& pVar = mtRecordVec.at(GetPos(row, i));
		if (nullptr == pVar)
		{
			pVar = SQUICK_SHARE_PTR<SquickData>(SQUICK_NEW SquickData(mVarRecordType->Type(i)));
		}

		pVar->variantData = mVarRecordType->GetStack(i)->variantData;
	}
	return true;
}

bool Record::ValidPos(int row, int col) const
{
    if (ValidCol(col)
        && ValidRow(row))
    {
        return true;
    }

    return false;
}

bool Record::ValidRow(int row) const
{
    if (row >= GetRows() || row < 0)
    {
        return false;
    }

    return true;
}

bool Record::ValidCol(int col) const
{
    if (col >= GetCols() || col < 0)
    {
        return false;
    }

    return true;
}

int Record::GetCol(const std::string& strTag) const
{
    std::map<std::string, int>::const_iterator it = mmTag.find(strTag);
    if (it != mmTag.end())
    {
        return it->second;
    }

    return -1;
}
