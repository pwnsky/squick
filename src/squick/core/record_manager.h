

#ifndef SQUICK_RECORDMANAGER_H
#define SQUICK_RECORDMANAGER_H

#include <map>
#include <list>
#include <string>
#include <vector>
#include "record.h"
#include "i_record_manager.h"
#include "platform.h"

class _SquickExport RecordManager : public IRecordManager
{
public:
    RecordManager(const Guid& self)
    {
        mSelf = self;
    }

    virtual ~RecordManager();

    virtual SQUICK_SHARE_PTR<IRecord> AddRecord(const Guid& self, const std::string& recordName, const SQUICK_SHARE_PTR<DataList>& TData, const SQUICK_SHARE_PTR<DataList>& tagData, const int nRows);

    virtual const Guid& Self();

	virtual std::string ToString();
    //////////////////////////////////////////////////////////////////////////

    virtual bool SetRecordInt(const std::string& recordName, const int row, const int col, const INT64 nValue);
    virtual bool SetRecordFloat(const std::string& recordName, const int row, const int col, const double dwValue);
    virtual bool SetRecordString(const std::string& recordName, const int row, const int col, const std::string& value);
    virtual bool SetRecordObject(const std::string& recordName, const int row, const int col, const Guid& obj);
	virtual bool SetRecordVector2(const std::string& recordName, const int row, const int col, const Vector2& obj);
	virtual bool SetRecordVector3(const std::string& recordName, const int row, const int col, const Vector3& obj);

    virtual bool SetRecordInt(const std::string& recordName, const int row, const std::string& colTag, const INT64 value);
    virtual bool SetRecordFloat(const std::string& recordName, const int row, const std::string& colTag, const double value);
    virtual bool SetRecordString(const std::string& recordName, const int row, const std::string& colTag, const std::string& value);
    virtual bool SetRecordObject(const std::string& recordName, const int row, const std::string& colTag, const Guid& value);
	virtual bool SetRecordVector2(const std::string& recordName, const int row, const std::string& colTag, const Vector2& value);
	virtual bool SetRecordVector3(const std::string& recordName, const int row, const std::string& colTag, const Vector3& value);

    virtual INT64 GetRecordInt(const std::string& recordName, const int row, const int col);
    virtual double GetRecordFloat(const std::string& recordName, const int row, const int col);
    virtual const std::string& GetRecordString(const std::string& recordName, const int row, const int col);
    virtual const Guid& GetRecordObject(const std::string& recordName, const int row, const int col);
	virtual const Vector2& GetRecordVector2(const std::string& recordName, const int row, const int col);
	virtual const Vector3& GetRecordVector3(const std::string& recordName, const int row, const int col);

    virtual INT64 GetRecordInt(const std::string& recordName, const int row, const std::string& colTag);
    virtual double GetRecordFloat(const std::string& recordName, const int row, const std::string& colTag);
    virtual const std::string& GetRecordString(const std::string& recordName, const int row, const std::string& colTag);
    virtual const Guid& GetRecordObject(const std::string& recordName, const int row, const std::string& colTag);
	virtual const Vector2& GetRecordVector2(const std::string& recordName, const int row, const std::string& colTag);
	virtual const Vector3& GetRecordVector3(const std::string& recordName, const int row, const std::string& colTag);
    //////////////////////////////////////////////////////////////////////////
private:
    Guid mSelf;

};

#endif
