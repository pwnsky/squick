

#ifndef SQUICK_PROPERTY_H
#define SQUICK_PROPERTY_H

#include "map.h"
#include "list.h"
#include "data_list.h"
#include "i_property.h"
#include "platform.h"

class _SquickExport Property : public IProperty
{
private:
    Property();

public:
    Property(const Guid& self, const std::string& propertyName, const DATA_TYPE varType);

    virtual ~Property();

    virtual void SetValue(const SquickData& TData);
    virtual void SetValue(const IProperty* property);

    virtual bool SetInt(const INT64 value, const INT64 reason = 0);
    virtual bool SetFloat(const double value, const INT64 reason = 0);
    virtual bool SetString(const std::string& value, const INT64 reason = 0);
    virtual bool SetObject(const Guid& value, const INT64 reason = 0);
	virtual bool SetVector2(const Vector2& value, const INT64 reason = 0);
	virtual bool SetVector3(const Vector3& value, const INT64 reason = 0);

    virtual const DATA_TYPE GetType() const;
    virtual const bool GeUsed() const;
    virtual const std::string& GetKey() const;
    virtual const bool GetSave() const;
    virtual const bool GetPublic() const;
    virtual const bool GetPrivate() const;
    virtual const bool GetCache() const;
	virtual const bool GetRef() const;
	virtual const bool GetForce() const;
	virtual const bool GetUpload() const;

    virtual void SetSave(bool bSave);
    virtual void SetPublic(bool bPublic);
    virtual void SetPrivate(bool bPrivate);
    virtual void SetCache(bool bCache);
	virtual void SetRef(bool bRef);
	virtual void SetForce(bool bRef);
	virtual void SetUpload(bool bUpload);

    virtual INT64 GetInt() const;
	virtual int GetInt32() const;
    virtual double GetFloat() const;
    virtual const std::string& GetString() const;
    virtual const Guid& GetObject() const;
	virtual const Vector2& GetVector2() const;
	virtual const Vector3& GetVector3() const;

    virtual const SquickData& GetValue() const;
    virtual const SQUICK_SHARE_PTR<List<std::string>> GetEmbeddedList() const;
    virtual const SQUICK_SHARE_PTR<MapEx<std::string, std::string>> GetEmbeddedMap() const;

    virtual bool Changed() const;

    virtual std::string ToString();
    virtual void ToMemoryCounterString(std::string& data);
    virtual bool FromString(const std::string& strData);
    virtual bool DeSerialization();

    virtual void RegisterCallback(const PROPERTY_EVENT_FUNCTOR_PTR& cb);

private:
    int OnEventHandler(const SquickData& oldVar, const SquickData& newVar, const INT64 reason);

private:
    typedef std::vector<PROPERTY_EVENT_FUNCTOR_PTR> TPROPERTYCALLBACKEX;
    TPROPERTYCALLBACKEX mtPropertyCallback;

    Guid mSelf;
    std::string msPropertyName;
    DATA_TYPE eType;

    SQUICK_SHARE_PTR<SquickData> mxData;
    SQUICK_SHARE_PTR<MapEx<std::string, std::string>> mxEmbeddedMap;
    SQUICK_SHARE_PTR<List<std::string>> mxEmbeddedList;

    bool mbPublic;
    bool mbPrivate;
    bool mbSave;
    bool mbCache;
	bool mbRef;
	bool mbForce;
	bool mbUpload;
};

#endif
