

#ifndef SQUICK_INTF_RECORDMANAGER_H
#define SQUICK_INTF_RECORDMANAGER_H

#include "i_record.h"
#include "map.h"
#include "platform.h"

class _SquickExport IRecordManager : public MapEx<std::string, IRecord> {
  public:
    virtual ~IRecordManager() {}

    virtual std::shared_ptr<IRecord> AddRecord(const Guid &self, const std::string &recordName, const std::shared_ptr<DataList> &TData,
                                               const std::shared_ptr<DataList> &tagData, const int nRows) = 0;
    virtual const Guid &Self() = 0;
    virtual std::string ToString() = 0;
    //////////////////////////////////////////////////////////////////////////

    virtual bool SetRecordInt(const std::string &recordName, const int row, const int col, const INT64 nValue) = 0;
    virtual bool SetRecordFloat(const std::string &recordName, const int row, const int col, const double dwValue) = 0;
    virtual bool SetRecordString(const std::string &recordName, const int row, const int col, const std::string &value) = 0;
    virtual bool SetRecordObject(const std::string &recordName, const int row, const int col, const Guid &obj) = 0;
    virtual bool SetRecordVector2(const std::string &recordName, const int row, const int col, const Vector2 &obj) = 0;
    virtual bool SetRecordVector3(const std::string &recordName, const int row, const int col, const Vector3 &obj) = 0;

    virtual bool SetRecordInt(const std::string &recordName, const int row, const std::string &colTag, const INT64 value) = 0;
    virtual bool SetRecordFloat(const std::string &recordName, const int row, const std::string &colTag, const double value) = 0;
    virtual bool SetRecordString(const std::string &recordName, const int row, const std::string &colTag, const std::string &value) = 0;
    virtual bool SetRecordObject(const std::string &recordName, const int row, const std::string &colTag, const Guid &value) = 0;
    virtual bool SetRecordVector2(const std::string &recordName, const int row, const std::string &colTag, const Vector2 &value) = 0;
    virtual bool SetRecordVector3(const std::string &recordName, const int row, const std::string &colTag, const Vector3 &value) = 0;

    virtual INT64 GetRecordInt(const std::string &recordName, const int row, const int col) = 0;
    virtual double GetRecordFloat(const std::string &recordName, const int row, const int col) = 0;
    virtual const std::string &GetRecordString(const std::string &recordName, const int row, const int col) = 0;
    virtual const Guid &GetRecordObject(const std::string &recordName, const int row, const int col) = 0;
    virtual const Vector2 &GetRecordVector2(const std::string &recordName, const int row, const int col) = 0;
    virtual const Vector3 &GetRecordVector3(const std::string &recordName, const int row, const int col) = 0;

    virtual INT64 GetRecordInt(const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual double GetRecordFloat(const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const std::string &GetRecordString(const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const Guid &GetRecordObject(const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const Vector2 &GetRecordVector2(const std::string &recordName, const int row, const std::string &colTag) = 0;
    virtual const Vector3 &GetRecordVector3(const std::string &recordName, const int row, const std::string &colTag) = 0;

    //////////////////////////////////////////////////////////////////////////
};

#endif
