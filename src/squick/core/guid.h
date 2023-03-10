

#ifndef SQUICK_IDENTID_H
#define SQUICK_IDENTID_H

#include "platform.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

class Guid
{
private:
	static INT64 nInstanceID;
	static INT64 nGUIDIndex;

public:

    INT64 nData64;
    INT64 nHead64;

	static void SetInstanceID(INT64 id)
	{
		/*
		if (nInstanceID != 0)
		{
			std::cout << "ERROR-------------------- set instance id again!!!" << std::endl;
			return;
		}
		*/
		nInstanceID = id;
		nGUIDIndex = 0;
	}

    Guid()
    {
        nData64 = 0;
        nHead64 = 0;
    }

    Guid(INT64 nHeadData, INT64 nData)
    {
        nHead64 = nHeadData;
        nData64 = nData;
    }

    Guid(const Guid& xData)
    {
        nHead64 = xData.nHead64;
        nData64 = xData.nData64;
    }
  
    Guid(const std::string& strID)
    {
        FromString(strID);
    }

	static Guid CreateID()
	{
		int64_t value = 0;
		uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();


		//value = time << 16;
		value = time * 1000000;


		//value |= nGUIDIndex++;
		value += nGUIDIndex++;

		//if (sequence_ == 0x7FFF)
		if (nGUIDIndex == 999999)
		{
			nGUIDIndex = 0;
		}

		Guid xID;
		xID.nHead64 = nInstanceID;
		xID.nData64 = value;

		return xID;
	}

    Guid& operator=(const Guid& xData)
    {
        nHead64 = xData.nHead64;
        nData64 = xData.nData64;

        return *this;
    }
  
    Guid& operator=(const std::string& strID)
    {
        FromString(strID);

        return *this;
    }

    const INT64 GetData() const
    {
        return nData64;
    }

    const INT64 GetHead() const
    {
        return nHead64;
    }

    void SetData(const INT64 nData)
    {
        nData64 = nData;
    }

    void SetHead(const INT64 nData)
    {
        nHead64 = nData;
    }

    bool IsNull() const
    {
        return 0 == nData64 && 0 == nHead64;
    }

    bool operator == (const Guid& id) const
    {
        return this->nData64 == id.nData64 && this->nHead64 == id.nHead64;
    }

    bool operator != (const Guid& id) const
    {
        return this->nData64 != id.nData64 || this->nHead64 != id.nHead64;
    }

    bool operator < (const Guid& id) const
    {
        if (this->nHead64 == id.nHead64)
        {
            return this->nData64 < id.nData64;
        }

        return this->nHead64 < id.nHead64;
    }

    std::string ToString() const
    {
        return lexical_cast<std::string>(nHead64) + "-" + lexical_cast<std::string>(nData64);
    }

    bool FromString(const std::string& strID)
    {
        size_t nStrLength = strID.length();
        size_t nPos = strID.find('-');
        if (nPos == std::string::npos)
        {
            return false;
        }

        std::string strHead = strID.substr(0, nPos);
        std::string strData = "";
        if (nPos + 1 < nStrLength)
        {
            strData = strID.substr(nPos + 1, nStrLength - nPos);
        }

        try
        {
            nHead64 = lexical_cast<INT64>(strHead);
            nData64 = lexical_cast<INT64>(strData);

            return true;
        }
        catch (...)
        {
            return false;
        }
    }
};
/*
INT64 Guid::nInstanceID = 0;
INT64 Guid::nGUIDIndex = 0;
*/
#endif
