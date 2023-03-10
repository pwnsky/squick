
#ifndef SQUICK_DATALIST_H
#define SQUICK_DATALIST_H

#include <cstdarg>
#include <string>
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>
#include <map>
#include <typeinfo>
#include <iomanip>
#include <functional>
#include <iostream>
#include <fstream>
#include <memory>
#include <variant>
#include "memory_counter.h"
#include "guid.h"
#include "platform.h"
#include "vector2.h"
#include "vector3.h"



enum DATA_TYPE
{
    TDATA_UNKNOWN,  
    TDATA_INT,      
    TDATA_FLOAT,    
    TDATA_STRING,   
    TDATA_OBJECT,   
	TDATA_VECTOR2,
	TDATA_VECTOR3,
    TDATA_MAX,
};

const static std::string NULL_STR = "";
const static Guid NULL_OBJECT = Guid();
const static double NULL_FLOAT = 0.0;
const static INT64 NULL_INT = 0;
const static Vector2 NULL_VECTOR2 = Vector2();
const static Vector3 NULL_VECTOR3 = Vector3();

struct SquickData
{
public:
	SquickData()
	{
		type = TDATA_UNKNOWN;
	}

	SquickData(DATA_TYPE eType)
	{
		bind = true;
		type = eType;
	}
	
	SquickData(int value)
	{
		bind = true;
		this->SetInt(value);
	}
	SquickData(int64_t value)
	{
		bind = true;
		this->SetInt(value);
	}

	SquickData(float value)
	{
		bind = true;
		this->SetFloat(value);
	}

	SquickData(double value)
	{
		bind = true;
		this->SetFloat(value);
	}

	SquickData(Guid value)
	{
		bind = true;
		this->SetObject(value);
	}

	SquickData(Vector2 value)
	{
		bind = true;
		this->SetVector2(value);
	}

	SquickData(Vector3 value)
	{
		bind = true;
		this->SetVector3(value);
	}

	SquickData(const SquickData& value)
	{
		bind = value.bind;
		type = value.type;
		variantData = value.variantData;
	}

	~SquickData()
	{
		type = TDATA_UNKNOWN;
	}

	inline bool operator==(const SquickData& src) const
	{
		//&& src.variantData == variantData
		if (src.GetType() == GetType())
		{
			switch (GetType())
			{
			case TDATA_INT:
			{
				if (src.GetInt() == GetInt())
				{
					return true;
				}
			}
			break;
			case TDATA_FLOAT:
			{
				double fValue = GetFloat() - src.GetFloat();
				if (fValue < 0.001  && fValue > -0.001)
				{
					return true;
				}
			}
			break;
			case TDATA_STRING:
			{
				if (src.GetString() == GetString())
				{
					return true;
				}
			}
			break;
			case TDATA_OBJECT:
			{
				if (src.GetObject() == GetObject())
				{
					return true;
				}
			}
			break;
			case TDATA_VECTOR2:
			{
				if (src.GetVector2() == GetVector2())
				{
					return true;
				}
			}
			break;
			case TDATA_VECTOR3:
			{
				if (src.GetVector3() == GetVector3())
				{
					return true;
				}
			}
			break;
			default:
				break;
			}
		}

		return false;
	}

	void Reset()
	{
		variantData = std::variant<INT64, double, std::string, Guid, Vector2, Vector3>();
		type = TDATA_UNKNOWN;
		/*switch (GetType())
		{
			case TDATA_INT:
			{
				SetInt(0);
			}
				break;
			case TDATA_FLOAT:
			{
				SetFloat(0);
			}
				break;
			case TDATA_STRING:
			{
				SetString("");
			}
				break;
			case TDATA_OBJECT:
			{
				SetObject(Guid());
			}
				break;
			case TDATA_VECTOR2:
			{
				SetVector2(Vector2());
			}
				break;
			case TDATA_VECTOR3:
			{
				SetVector3(Vector3());
			}
				break;
			default:
				break;
		}*/
	}

	bool IsNullValue() const
	{
		bool bChanged = false;

		switch (GetType())
		{
		case TDATA_INT:
		{
			if (0 != GetInt())
			{
				bChanged = true;
			}
		}
		break;
		case TDATA_FLOAT:
		{
			double fValue = GetFloat();
			if (fValue > 0.001 || fValue < -0.001)
			{
				bChanged = true;
			}
		}
		break;
		case TDATA_STRING:
		{
			const std::string& strData = GetString();
			if (!strData.empty())
			{
				bChanged = true;
			}
		}
		break;
		case TDATA_OBJECT:
		{
			if (!GetObject().IsNull())
			{
				bChanged = true;
			}
		}
		break;
		case TDATA_VECTOR2:
		{
			if (!GetVector2().IsZero())
			{
				bChanged = true;
			}
		}
		break;
		case TDATA_VECTOR3:
		{
			if (!GetVector3().IsZero())
			{
				bChanged = true;
			}
		}
		break;
		default:
			break;
		}

		return !bChanged;
	}

	DATA_TYPE GetType() const
	{
		return type;
	}


	void SetInt(const INT64 var)
	{
		if (type == TDATA_INT || TDATA_UNKNOWN == type)
		{
			type = TDATA_INT;
			variantData = (INT64)var;
		}
	}

	void SetFloat(const double var)
	{
		if (type == TDATA_FLOAT || TDATA_UNKNOWN == type)
		{
			type = TDATA_FLOAT;
			variantData = (double)var;
		}
	}

	void SetString(const std::string& var)
	{
		if (type == TDATA_STRING || TDATA_UNKNOWN == type)
		{
			type = TDATA_STRING;
			variantData = (std::string)var;
		}
	}

	void SetObject(const Guid var)
	{
		if (type == TDATA_OBJECT || TDATA_UNKNOWN == type)
		{
			type = TDATA_OBJECT;
			variantData = (Guid)var;
		}
	}

	void SetVector2(const Vector2 var)
	{
		if (type == TDATA_VECTOR2 || TDATA_UNKNOWN == type)
		{
			type = TDATA_VECTOR2;
			variantData = (Vector2)var;
		}
	}

	void SetVector3(const Vector3 var)
	{
		if (type == TDATA_VECTOR3 || TDATA_UNKNOWN == type)
		{
			type = TDATA_VECTOR3;
			variantData = (Vector3)var;
		}
	}

	INT64 GetInt() const
	{
		if (TDATA_INT == type)
		{
			return std::get<INT64>(variantData);
		}

		return NULL_INT;
	}

	int GetInt32() const
	{
		if (TDATA_INT == type)
		{
			return (int)std::get<INT64>(variantData);
		}

		return (int)NULL_INT;
	}

	double GetFloat() const
	{
		if (TDATA_FLOAT == type)
		{
			return std::get<double>(variantData);
		}

		return NULL_FLOAT;
	}
	const std::string& GetString() const
	{
		if (TDATA_STRING == type)
		{
			return std::get<std::string>(variantData);
		}

		return NULL_STR;
	}

	const Guid& GetObject() const
	{
		if (TDATA_OBJECT == type)
		{
			return std::get<Guid>(variantData);
		}

		return NULL_OBJECT;
	}

	const Vector2& GetVector2() const
	{
		if (TDATA_VECTOR2 == type)
		{
			return std::get<Vector2>(variantData);
		}

		return NULL_VECTOR2;
	}

	const Vector3& GetVector3() const
	{
		if (TDATA_VECTOR3 == type)
		{
			return std::get<Vector3>(variantData);
		}

		return NULL_VECTOR3;
	}

	std::string ToString() const
	{
		std::string strData;

		switch (type)
		{
		case TDATA_INT:
			strData = lexical_cast<std::string> (GetInt());
			break;

		case TDATA_FLOAT:
			strData = lexical_cast<std::string> (GetFloat());
			break;

		case TDATA_STRING:
			strData = GetString();
			break;

		case TDATA_OBJECT:
			strData = GetObject().ToString();
			break;

		case TDATA_VECTOR2:
			strData = GetVector2().ToString();
			break;

		case TDATA_VECTOR3:
			strData = GetVector3().ToString();
			break;

		default:
			strData = NULL_STR;
			break;
		}
		return strData;
	}

	bool FromString(const std::string& strData)
	{
		try
		{
			switch (type)
			{
			case TDATA_INT:
				SetInt(lexical_cast<int64_t> (strData));
				break;

			case TDATA_FLOAT:
				SetFloat(lexical_cast<float> (strData));
				break;

			case TDATA_STRING:
				SetString(strData);
				break;

			case TDATA_OBJECT:
			{
				Guid xID;
				xID.FromString(strData);
				SetObject(xID);
			}
			break;

			case TDATA_VECTOR2:
			{
				Vector2 v;
				v.FromString(strData);
				SetVector2(v);
			}
			break;

			case TDATA_VECTOR3:
			{
				Vector3 v;
				v.FromString(strData);
				SetVector3(v);
			}
			break;

			default:
				break;
			}

			return true;
		}
		catch (...)
		{
			return false;
		}
		
		return false;
	}

private:

	DATA_TYPE type;
	bool bind = false;

public:
	std::variant<INT64, double, std::string, Guid, Vector2, Vector3> variantData;
	//mapbox::util::variant<INT64, double, std::string, Guid, Vector2, Vector3> variantData;
};

class DataList :public MemoryCounter
{
public:
    DataList() : MemoryCounter(GET_CLASS_NAME(DataList))
    {
        mnUseSize = 0;
        mvList.reserve(STACK_SIZE);
        for (int i = 0; i < STACK_SIZE; ++i)
        {
            mvList.push_back(SQUICK_SHARE_PTR<SquickData>(SQUICK_NEW SquickData()));
        }
    }

	DataList(const std::string& data, const std::string& strSplit) : MemoryCounter(GET_CLASS_NAME(DataList))
	{
		mnUseSize = 0;
		mvList.reserve(STACK_SIZE);
		for (int i = 0; i < STACK_SIZE; ++i)
		{
			mvList.push_back(SQUICK_SHARE_PTR<SquickData>(SQUICK_NEW SquickData()));
		}

		this->Split(data, strSplit);
	}

	virtual ~DataList()
	{
	}

	static const DataList& Empty()
	{
		static DataList data;
		return data;
	}

	virtual std::string ToString() const
	{
		std::stringstream os;
		for (int i = 0; i < GetCount(); ++i)
		{
			os << ToString(i);
			if (i < GetCount() - 1)
			{
				os << "|";
			}
		}

		return os.str();
	}

	virtual void ToMemoryCounterString(std::string& data)
	{
		
	}

	virtual std::string ToString(const int index) const
	{
		std::string strData;

		if (ValidIndex(index))
		{
			const DATA_TYPE eType = Type(index);
			switch (eType)
			{
			case TDATA_INT:
				strData = lexical_cast<std::string> (Int(index));
				break;

			case TDATA_FLOAT:
				strData = lexical_cast<std::string> (Float(index));
				break;

			case TDATA_STRING:
				strData = String(index);
				break;

			case TDATA_OBJECT:
				strData = Object(index).ToString();
				break;

			case TDATA_VECTOR2:
				strData = Vector2At(index).ToString();
				break;

			case TDATA_VECTOR3:
				strData = Vector3At(index).ToString();
				break;

			default:
				strData = NULL_STR;
				break;
			}
		}

		return strData;
	}

	virtual bool ToString(std::string& str, const std::string& strSplit) const
	{
		for (int i = 0; i < GetCount(); ++i)
		{
			std::string strVal = ToString(i);
			str += strVal;
			str += strSplit;
		}

		std::string strTempSplit(strSplit);
		std::string::size_type nPos = str.rfind(strSplit);
		if (nPos == str.length() - strTempSplit.length())
		{
			str = str.substr(0, nPos);
		}

		return true;
	}

public:

	virtual const SQUICK_SHARE_PTR<SquickData> GetStack(const int index) const
	{
		if (index < (int)mvList.size())
		{
			return mvList[index];
		}

		return SQUICK_SHARE_PTR<SquickData>();
	}

	virtual bool Concat(const DataList& src)
	{
		InnerAppendEx(src, 0, src.GetCount());
		return true;
	}
    
	virtual bool Append(const DataList& src)
	{
		return Append(src, 0, src.GetCount());
	}

	virtual bool Append(const DataList& src, const int start, const int count)
	{
		if (start >= src.GetCount())
		{
			return false;
		}

		int end = start + count;

		if (end > src.GetCount())
		{
			return false;
		}

		InnerAppendEx(src, start, end);

		return true;
	}
    
	virtual bool Append(const SquickData& xData)
	{
		if (xData.GetType() <= TDATA_UNKNOWN
			|| xData.GetType() >= TDATA_MAX)
		{
			return false;
		}

		switch (xData.GetType())
		{
		case TDATA_INT:
			AddInt(xData.GetInt());
			break;
		case TDATA_FLOAT:
			AddFloat(xData.GetFloat());
			break;
		case TDATA_OBJECT:
			AddObject(xData.GetObject());
			break;
		case TDATA_STRING:
			AddString(xData.GetString());
			break;
		case TDATA_VECTOR2:
			AddVector2(xData.GetVector2());
			break;
		case TDATA_VECTOR3:
			AddVector3(xData.GetVector3());
			break;
		default:
			break;
		}

		return true;
	}
    
	virtual void Clear()
	{
		mnUseSize = 0;

		if (mvList.size() > STACK_SIZE)
		{
			mvList.erase(mvList.begin() + STACK_SIZE, mvList.end());
		}
		for (int i = 0; i < STACK_SIZE; ++i)
		{
			mvList[i]->Reset();
		}
	}
    
	virtual bool IsEmpty() const
	{
		return (0 == mnUseSize);
	}
    
	virtual int GetCount() const
	{
		return mnUseSize;
	}
    
	virtual DATA_TYPE Type(const int index) const
	{
		if (!ValidIndex(index))
		{
			return TDATA_UNKNOWN;
		}

		if (index < STACK_SIZE)
		{
			return mvList[index]->GetType();
		}
		else
		{
			const SQUICK_SHARE_PTR<SquickData> pData = GetStack(index);
			if (pData)
			{
				return pData->GetType();
			}
		}

		return TDATA_UNKNOWN;
	}
    
	virtual bool TypeEx(const  int type, ...) const
	{

		bool bRet = true;

		if (TDATA_UNKNOWN == type)
		{
			bRet = false;
			return bRet;
		}

		DATA_TYPE pareType = (DATA_TYPE)type;
		va_list arg_ptr;
		va_start(arg_ptr, type);
		int index = 0;

		while (pareType != TDATA_UNKNOWN)
		{

			DATA_TYPE varType = Type(index);
			if (varType != pareType)
			{
				bRet = false;
				break;
			}

			++index;
			pareType = (DATA_TYPE)va_arg(arg_ptr, int);
		}

		va_end(arg_ptr);

		return bRet;
	}
    
	virtual bool Split(const std::string& str, const std::string& strSplit)
	{
		Clear();

		std::string strData(str);
		if (strData.empty())
		{
			return true;
		}

		std::string temstrSplit(strSplit);
		std::string::size_type pos;
		strData += temstrSplit;
		std::string::size_type size = strData.length();

		for (std::string::size_type i = 0; i < size; i++)
		{
			pos = int(strData.find(temstrSplit, i));
			if (pos < size)
			{
				std::string strSub = strData.substr(i, pos - i);
				Add(strSub.c_str());

				i = pos + temstrSplit.size() - 1;
			}
		}

		return true;
	}

    
	virtual bool Add(const INT64 value)
	{
		if (GetCount() == (int)(mvList.size()))
		{
			AddStatck();
		}

		SQUICK_SHARE_PTR<SquickData> var = GetStack(GetCount());
		if (var)
		{
			var->SetInt(value);
			mnUseSize++;

			return true;
		}

		return false;
	}

	virtual bool Add(const double value)
	{
		if (GetCount() == (int)(mvList.size()))
		{
			AddStatck();
		}

		SQUICK_SHARE_PTR<SquickData> var = GetStack(GetCount());
		if (var)
		{
			var->SetFloat(value);
			mnUseSize++;

			return true;
		}

		return false;
	}

	virtual bool Add(const std::string& value)
	{
		if (GetCount() == (int)(mvList.size()))
		{
			AddStatck();
		}

		SQUICK_SHARE_PTR<SquickData> var = GetStack(GetCount());
		if (var)
		{
			var->SetString(value);
			mnUseSize++;

			return true;
		}

		return false;
	}

	virtual bool Add(const Guid& value)
	{
		if (GetCount() == (int)(mvList.size()))
		{
			AddStatck();
		}

		SQUICK_SHARE_PTR<SquickData> var = GetStack(GetCount());
		if (var)
		{
			var->SetObject(value);
			mnUseSize++;

			return true;
		}

		return false;
	}

	virtual bool Add(const Vector2& value)
	{
		if (GetCount() == (int)(mvList.size()))
		{
			AddStatck();
		}

		SQUICK_SHARE_PTR<SquickData> var = GetStack(GetCount());
		if (var)
		{
			var->SetVector2(value);
			mnUseSize++;

			return true;
		}

		return false;
	}

	virtual bool Add(const Vector3& value)
	{
		if (GetCount() == (int)(mvList.size()))
		{
			AddStatck();
		}

		SQUICK_SHARE_PTR<SquickData> var = GetStack(GetCount());
		if (var)
		{
			var->SetVector3(value);
			mnUseSize++;

			return true;
		}

		return false;
	}

	virtual bool Set(const int index, const INT64 value)
	{
		if (ValidIndex(index) && Type(index) == TDATA_INT)
		{
			SQUICK_SHARE_PTR<SquickData> var = GetStack(index);
			if (var)
			{
				var->SetInt(value);

				return true;
			}
		}

		return false;
	}

	virtual bool Set(const int index, const double value)
	{
		if (ValidIndex(index) && Type(index) == TDATA_FLOAT)
		{
			SQUICK_SHARE_PTR<SquickData> var = GetStack(index);
			if (var)
			{
				var->SetFloat(value);

				return true;
			}
		}

		return false;
	}

	virtual bool Set(const int index, const std::string& value)
	{
		if (ValidIndex(index) && Type(index) == TDATA_STRING)
		{
			SQUICK_SHARE_PTR<SquickData> var = GetStack(index);
			if (var)
			{
				var->SetString(value);

				return true;
			}
		}

		return false;
	}

	virtual bool Set(const int index, const Guid& value)
	{
		if (ValidIndex(index) && Type(index) == TDATA_OBJECT)
		{
			SQUICK_SHARE_PTR<SquickData> var = GetStack(index);
			if (var)
			{
				var->SetObject(value);

				return true;
			}
		}

		return false;
	}

	virtual bool Set(const int index, const Vector2& value)
	{
		if (ValidIndex(index) && Type(index) == TDATA_VECTOR2)
		{
			SQUICK_SHARE_PTR<SquickData> var = GetStack(index);
			if (var)
			{
				var->SetVector2(value);

				return true;
			}
		}

		return false;
	}

	virtual bool Set(const int index, const Vector3& value)
	{
		if (ValidIndex(index) && Type(index) == TDATA_VECTOR3)
		{
			SQUICK_SHARE_PTR<SquickData> var = GetStack(index);
			if (var)
			{
				var->SetVector3(value);

				return true;
			}
		}

		return false;
	}

    
	virtual INT64 Int(const int index) const
	{
		if (ValidIndex(index))
		{
			if (Type(index) == TDATA_INT)
			{
				const SQUICK_SHARE_PTR<SquickData> var = GetStack(index);
				return var->GetInt();
			}
		}

		return 0;
	}

	inline int Int32(const int index) const
	{
		return (int) Int(index);
	}

	virtual double Float(const int index) const
	{
		if (ValidIndex(index))
		{
			const SQUICK_SHARE_PTR<SquickData> var = mvList[index];
			if (var && TDATA_FLOAT == var->GetType())
			{
				return var->GetFloat();
			}
		}

		return 0.0f;
	}

	virtual const std::string& String(const int index) const
	{
		if (ValidIndex(index))
		{
			const SQUICK_SHARE_PTR<SquickData> var = mvList[index];
			if (var && TDATA_STRING == var->GetType())
			{
				return var->GetString();
			}
		}

		return NULL_STR;
	}

	virtual const Guid& Object(const int index) const
	{
		if (ValidIndex(index))
		{
			DATA_TYPE type = Type(index);
			if (TDATA_OBJECT == type)
			{
				SQUICK_SHARE_PTR<SquickData> var = GetStack(index);
				if (var)
				{
					return var->GetObject();
				}
			}
		}

		return NULL_OBJECT;
	}

	virtual const Vector2& Vector2At(const int index) const
	{
		if (ValidIndex(index))
		{
			DATA_TYPE type = Type(index);
			if (TDATA_VECTOR2 == type)
			{
				SQUICK_SHARE_PTR<SquickData> var = GetStack(index);
				if (var)
				{
					return var->GetVector2();
				}
			}
		}

		return NULL_VECTOR2;
	}

	virtual const Vector3& Vector3At(const int index) const
	{
		if (ValidIndex(index))
		{
			DATA_TYPE type = Type(index);
			if (TDATA_VECTOR3 == type)
			{
				SQUICK_SHARE_PTR<SquickData> var = GetStack(index);
				if (var)
				{
					return var->GetVector3();
				}
			}
		}

		return NULL_VECTOR3;
	}

    bool AddInt(const INT64 value)
    {
        return Add(value);
    }
    bool AddFloat(const double value)
    {
        return Add(value);
    }
    bool AddString(const std::string& value)
    {
        return Add(value);
    }
    bool AddStringFromChar(const char* value)
    {
        return Add(value);
    }
    bool AddObject(const Guid& value)
    {
        return Add(value);
    }
	bool AddVector2(const Vector2& value)
	{
		return Add(value);
	}
	bool AddVector3(const Vector3& value)
	{
		return Add(value);
	}

    bool SetInt(const int index, const INT64 value)
    {
        return Set(index, value);
    }
    bool SetFloat(const int index, const double value)
    {
        return Set(index, value);
    }
    bool SetString(const int index, const std::string& value)
    {
        return Set(index, value);
    }
    bool SetObject(const int index, const Guid& value)
    {
        return Set(index, value);
    }
	bool SetVector2(const int index, const Vector2& value)
	{
		return Set(index, value);
	}
	bool SetVector3(const int index, const Vector3& value)
	{
		return Set(index, value);
	}



    inline bool Compare(const int nPos, const DataList& src) const
    {
        if (src.GetCount() > nPos
            && GetCount() > nPos
            && src.Type(nPos) == Type(nPos))
        {
            switch (src.Type(nPos))
            {
                case TDATA_INT:
                    return Int(nPos) == src.Int(nPos);
                    break;

                case TDATA_FLOAT:
                    return fabs(Float(nPos) - src.Float(nPos)) < 0.001f;
                    break;

                case TDATA_STRING:
                    return String(nPos) == src.String(nPos);
                    break;

                case TDATA_OBJECT:
                    return Object(nPos) == src.Object(nPos);
                    break;

				case TDATA_VECTOR2:
					return Vector2At(nPos) == src.Vector2At(nPos);
					break;

				case TDATA_VECTOR3:
					return Vector3At(nPos) == src.Vector3At(nPos);
					break;

                default:
                    return false;
                    break;
            }
        }

        return false;
    }


    inline bool operator==(const DataList& src) const
    {
        if (src.GetCount() == GetCount())
        {
            for (int i = 0; i < GetCount(); i++)
            {
                if (!Compare(i, src))
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    inline bool operator!=(const DataList& src)
    {
        return !(*this == src);
    }
    inline DataList& operator<<(const double value)
    {
        Add(value);
        return *this;
    }
    inline DataList& operator<<(const char* value)
    {
        Add(value);
        return *this;
    }
    inline DataList& operator<<(const std::string& value)
    {
        Add(value);
        return *this;
    }

    inline DataList& operator<<(const INT64& value)
    {
        Add(value);
        return *this;
    }
    inline DataList& operator<<(const int value)
    {
        Add((INT64)value);
        return *this;
    }
    inline DataList& operator<<(const Guid& value)
    {
        Add(value);
        return *this;
    }
	inline DataList& operator<<(const Vector2& value)
	{
		Add(value);
		return *this;
	}
	inline DataList& operator<<(const Vector3& value)
	{
		Add(value);
		return *this;
	}
    inline DataList& operator<<(const DataList& value)
    {
        Concat(value);
        return *this;
    }
    enum { STACK_SIZE = 8 };

protected:

	bool ValidIndex(int index) const
	{
		return (index < GetCount()) && (index >= 0);
	}

	void AddStatck()
	{
		for (int i = 0; i < STACK_SIZE; ++i)
		{
			SQUICK_SHARE_PTR<SquickData> pData(SQUICK_NEW  SquickData());
			mvList.push_back(pData);
		}
	}

	void InnerAppendEx(const DataList& src, const int start, const int end)
	{
		for (int i = start; i < end; ++i)
		{
			DATA_TYPE vType = src.Type(i);
			switch (vType)
			{
			case TDATA_INT:
				AddInt(src.Int(i));
				break;
			case TDATA_FLOAT:
				AddFloat(src.Float(i));
				break;
			case TDATA_STRING:
				AddString(src.String(i));
				break;
			case TDATA_OBJECT:
				AddObject(src.Object(i));
				break;
			case TDATA_VECTOR2:
				AddVector2(src.Vector2At(i));
				break;
			case TDATA_VECTOR3:
				AddVector3(src.Vector3At(i));
				break;
			default:
				break;
			}
		}
	}
protected:

    int mnUseSize;
    std::vector< SQUICK_SHARE_PTR<SquickData> > mvList;
    std::map<std::string, SQUICK_SHARE_PTR<SquickData> > mxMap;
};

const static SquickData NULL_TDATA = SquickData();

#endif