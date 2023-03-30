

#ifndef SQUICK_INTF_RECORD_H
#define SQUICK_INTF_RECORD_H

#include "data_list.h"
#include "platform.h"

class IRecord;

struct RECORD_EVENT_DATA {
    enum RecordOptype {
        Add = 0,
        Del, // before del
        AfterDel,
        Swap,
        Create,
        Update,
        Cleared,
        Sort,
        Cover,

        UNKNOW,
    };
    RECORD_EVENT_DATA() {
        nOpType = UNKNOW;
        row = 0;
        col = 0;
    }

    RecordOptype nOpType;
    int row;
    int col;
    std::string recordName;
    IRecord *recordData;
};

typedef std::function<int(const Guid &, const RECORD_EVENT_DATA &, const SquickData &, const SquickData &)> RECORD_EVENT_FUNCTOR;
typedef std::shared_ptr<RECORD_EVENT_FUNCTOR> RECORD_EVENT_FUNCTOR_PTR;

class _SquickExport IRecord : public MemoryCounter {
  public:
    IRecord() : MemoryCounter(GET_CLASS_NAME(IRecord), 1) {}

    typedef std::vector<std::shared_ptr<SquickData>> TRECORDVEC;
    typedef TRECORDVEC::const_iterator TRECORDVECCONSTITER;

    virtual ~IRecord() {}

    virtual std::string ToString() = 0;
    virtual bool IsUsed(const int row) const = 0;
    virtual bool SetUsed(const int row, const int bUse) = 0;
    virtual bool PreAllocMemoryForRow(const int row) = 0;

    virtual int GetCols() const = 0;
    virtual int GetRows() const = 0;
    virtual int GetUsedRows() const = 0;

    virtual DATA_TYPE GetColType(const int col) const = 0;
    virtual const std::string &GetColTag(const int col) const = 0;

    virtual int AddRow(const int row) = 0;
    virtual int AddRow(const int row, const DataList &var) = 0;

    virtual bool SetRow(const int row, const DataList &var) = 0;

    virtual bool SetInt(const int row, const int col, const INT64 value) = 0;
    virtual bool SetFloat(const int row, const int col, const double value) = 0;
    virtual bool SetString(const int row, const int col, const std::string &value) = 0;
    virtual bool SetObject(const int row, const int col, const Guid &value) = 0;
    virtual bool SetVector2(const int row, const int col, const Vector2 &value) = 0;
    virtual bool SetVector3(const int row, const int col, const Vector3 &value) = 0;

    virtual bool SetInt(const int row, const std::string &colTag, const INT64 value) = 0;
    virtual bool SetFloat(const int row, const std::string &colTag, const double value) = 0;
    virtual bool SetString(const int row, const std::string &colTag, const std::string &value) = 0;
    virtual bool SetObject(const int row, const std::string &colTag, const Guid &value) = 0;
    virtual bool SetVector2(const int row, const std::string &colTag, const Vector2 &value) = 0;
    virtual bool SetVector3(const int row, const std::string &colTag, const Vector3 &value) = 0;

    virtual bool QueryRow(const int row, DataList &varList) = 0;
    virtual bool SwapRowInfo(const int nOriginRow, const int nTargetRow) = 0;

    virtual INT64 GetInt(const int row, const int col) const = 0;
    virtual int GetInt32(const int row, const int col) const { return (int)GetInt(row, col); };
    virtual double GetFloat(const int row, const int col) const = 0;
    virtual const std::string &GetString(const int row, const int col) const = 0;
    virtual const Guid &GetObject(const int row, const int col) const = 0;
    virtual const Vector2 &GetVector2(const int row, const int col) const = 0;
    virtual const Vector3 &GetVector3(const int row, const int col) const = 0;

    virtual INT64 GetInt(const int row, const std::string &colTag) const = 0;
    inline int GetInt32(const int row, const std::string &colTag) const { return (int)GetInt(row, colTag); };
    virtual double GetFloat(const int row, const std::string &colTag) const = 0;
    virtual const std::string &GetString(const int row, const std::string &colTag) const = 0;
    virtual const Guid &GetObject(const int row, const std::string &colTag) const = 0;
    virtual const Vector2 &GetVector2(const int row, const std::string &colTag) const = 0;
    virtual const Vector3 &GetVector3(const int row, const std::string &colTag) const = 0;

    virtual int FindRowByColValue(const int col, const SquickData &var, DataList &varResult) = 0;
    virtual int FindInt(const int col, const INT64 value, DataList &varResult) = 0;
    virtual int FindFloat(const int col, const double value, DataList &varResult) = 0;
    virtual int FindString(const int col, const std::string &value, DataList &varResult) = 0;
    virtual int FindObject(const int col, const Guid &value, DataList &varResult) = 0;
    virtual int FindVector2At(const int col, const Vector2 &value, DataList &varResult) = 0;
    virtual int FindVector3At(const int col, const Vector3 &value, DataList &varResult) = 0;

    // return the row and only can used when the col is the primary key
    virtual int FindRowByColValue(const int col, const SquickData &var) = 0;
    virtual int FindInt(const int col, const INT64 value) = 0;
    virtual int FindFloat(const int col, const double value) = 0;
    virtual int FindString(const int col, const std::string &value) = 0;
    virtual int FindObject(const int col, const Guid &value) = 0;
    virtual int FindVector2At(const int col, const Vector2 &value) = 0;
    virtual int FindVector3At(const int col, const Vector3 &value) = 0;

    virtual int FindRowByColValue(const std::string &colTag, const SquickData &var, DataList &varResult) = 0;
    virtual int FindInt(const std::string &colTag, const INT64 value, DataList &varResult) = 0;
    virtual int FindFloat(const std::string &colTag, const double value, DataList &varResult) = 0;
    virtual int FindString(const std::string &colTag, const std::string &value, DataList &varResult) = 0;
    virtual int FindObject(const std::string &colTag, const Guid &value, DataList &varResult) = 0;
    virtual int FindVector2At(const std::string &colTag, const Vector2 &value, DataList &varResult) = 0;
    virtual int FindVector3At(const std::string &colTag, const Vector3 &value, DataList &varResult) = 0;

    // return the row and only can used when the col is the primary key
    virtual int FindRowByColValue(const std::string &colTag, const SquickData &var) = 0;
    virtual int FindInt(const std::string &colTag, const INT64 value) = 0;
    virtual int FindFloat(const std::string &colTag, const double value) = 0;
    virtual int FindString(const std::string &colTag, const std::string &value) = 0;
    virtual int FindObject(const std::string &colTag, const Guid &value) = 0;
    virtual int FindVector2At(const std::string &colTag, const Vector2 &value) = 0;
    virtual int FindVector3At(const std::string &colTag, const Vector3 &value) = 0;

    virtual bool Remove(const int row) = 0;
    virtual bool Remove(DataList &varRows) // need to optimize
    {
        for (int i = 0; i < varRows.GetCount(); ++i) {
            Remove((int)varRows.Int(i));
        }

        return true;
    }

    virtual bool Clear() = 0;

    virtual void AddRecordHook(const RECORD_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual const bool GetSave() = 0;
    virtual const bool GetPublic() = 0;
    virtual const bool GetPrivate() = 0;
    virtual const bool GetCache() = 0;
    virtual const bool GetRef() = 0;
    virtual const bool GetForce() = 0;
    virtual const bool GetUpload() = 0;
    virtual const std::string &GetName() const = 0;

    virtual std::shared_ptr<DataList> GetStartData() const = 0;
    virtual const std::shared_ptr<DataList> GetTag() const = 0;

    virtual void SetSave(const bool bSave) = 0;
    virtual void SetCache(const bool bCache) = 0;
    virtual void SetRef(const bool bRef) = 0;
    virtual void SetForce(const bool bForce) = 0;
    virtual void SetUpload(const bool bUpload) = 0;
    virtual void SetPublic(const bool bPublic) = 0;
    virtual void SetPrivate(const bool bPrivate) = 0;
    virtual void SetName(const std::string &name) = 0;

    virtual const TRECORDVEC &GetRecordVec() const = 0;
};

#endif
