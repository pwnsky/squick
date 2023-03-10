

#ifndef SQUICK_INTF_PROPERTY_H
#define SQUICK_INTF_PROPERTY_H

#include "data_list.h"
#include "list.h"
#include "platform.h"

typedef std::function<int(const Guid&, const std::string&, const SquickData&, const SquickData&, const INT64)> PROPERTY_EVENT_FUNCTOR;
typedef SQUICK_SHARE_PTR<PROPERTY_EVENT_FUNCTOR> PROPERTY_EVENT_FUNCTOR_PTR;


class _SquickExport IProperty : public MemoryCounter
{
public:
	IProperty() : MemoryCounter(GET_CLASS_NAME(IProperty), 1)
	{
	}

	virtual ~IProperty() {}

	virtual void SetValue(const SquickData& TData) = 0;
	virtual void SetValue(const IProperty* property) = 0;

	virtual bool SetInt(const INT64 value, const INT64 reason = 0) = 0;
	virtual bool SetFloat(const double value, const INT64 reason = 0) = 0;
	virtual bool SetString(const std::string& value, const INT64 reason = 0) = 0;
	virtual bool SetObject(const Guid& value, const INT64 reason = 0) = 0;
	virtual bool SetVector2(const Vector2& value, const INT64 reason = 0) = 0;
	virtual bool SetVector3(const Vector3& value, const INT64 reason = 0) = 0;

	virtual const DATA_TYPE GetType() const = 0;
	virtual const bool GeUsed() const = 0;
	virtual const std::string& GetKey() const = 0;
	virtual const bool GetSave() const = 0;
	virtual const bool GetPublic() const = 0;
	virtual const bool GetPrivate() const = 0;
	virtual const bool GetCache() const = 0;
	virtual const bool GetRef() const = 0;
	virtual const bool GetForce() const = 0;
	virtual const bool GetUpload() const = 0;

	virtual void SetSave(bool bSave) = 0;
	virtual void SetPublic(bool bPublic) = 0;
	virtual void SetPrivate(bool bPrivate) = 0;
	virtual void SetCache(bool bCache) = 0;
	virtual void SetRef(bool bRef) = 0;
	virtual void SetForce(bool bRef) = 0;
	virtual void SetUpload(bool bUpload) = 0;

	virtual INT64 GetInt() const = 0;
	virtual int GetInt32() const = 0;
	virtual double GetFloat() const = 0;
	virtual const std::string& GetString() const = 0;
	virtual const Guid& GetObject() const = 0;
	virtual const Vector2& GetVector2() const = 0;
	virtual const Vector3& GetVector3() const = 0;

	virtual const SquickData& GetValue() const = 0;
	virtual const SQUICK_SHARE_PTR<List<std::string>> GetEmbeddedList() const = 0;
	virtual const SQUICK_SHARE_PTR<MapEx<std::string, std::string>> GetEmbeddedMap() const = 0;

	virtual bool Changed() const = 0;

	virtual std::string ToString() = 0;
	virtual bool FromString(const std::string& strData) = 0;
	virtual bool DeSerialization() = 0;

	virtual void RegisterCallback(const PROPERTY_EVENT_FUNCTOR_PTR& cb) = 0;
};

#endif
