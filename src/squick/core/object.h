

#ifndef SQUICK_OBJECT_H
#define SQUICK_OBJECT_H

#include <iostream>
#include <string>
#include "i_object.h"
#include "i_record_manager.h"
#include "i_property_manager.h"
#include "platform.h"

class _SquickExport Object
    : public IObject
{
private:
    Object() : IObject(Guid())
    {
		mObjectEventState = COE_CREATE_NODATA;
    }

public:
    Object(Guid self, IPluginManager* pLuginManager);
    virtual ~Object();

    ///////////////////////////////////////////////////////////////////////

    virtual Guid Self() override ;

	virtual const Vector3& GetPosition() override ;
	virtual void SetPosition(const Vector3& pos) override ;

    virtual void ToMemoryCounterString(std::string& info) override ;
    /////////////////////////////////////////////////////////////////

	virtual CLASS_OBJECT_EVENT GetState() override ;
	virtual void SetState(const CLASS_OBJECT_EVENT eState) override ;
    virtual bool ObjectReady() override ;

    virtual bool FindProperty(const std::string& propertyName) override ;
    virtual SQUICK_SHARE_PTR<IProperty> AddProperty(const std::string& propertyName, const DATA_TYPE varType) override;

    virtual bool SetPropertyInt(const std::string& propertyName, const INT64 nValue, const INT64 reason = 0) override ;
    virtual bool SetPropertyFloat(const std::string& propertyName, const double dwValue, const INT64 reason = 0) override ;
    virtual bool SetPropertyString(const std::string& propertyName, const std::string& value, const INT64 reason = 0) override ;
    virtual bool SetPropertyObject(const std::string& propertyName, const Guid& obj, const INT64 reason = 0) override ;
	virtual bool SetPropertyVector2(const std::string& propertyName, const Vector2& value, const INT64 reason = 0) override ;
	virtual bool SetPropertyVector3(const std::string& propertyName, const Vector3& value, const INT64 reason = 0) override ;

    virtual INT64 GetPropertyInt(const std::string& propertyName) override ;
	virtual int GetPropertyInt32(const std::string& propertyName) override ;
    virtual double GetPropertyFloat(const std::string& propertyName) override ;
    virtual const std::string& GetPropertyString(const std::string& propertyName) override ;
    virtual const Guid& GetPropertyObject(const std::string& propertyName) override ;
	virtual const Vector2& GetPropertyVector2(const std::string& propertyName) override ;
	virtual const Vector3& GetPropertyVector3(const std::string& propertyName) override ;

    virtual SQUICK_SHARE_PTR<IRecord> FindRecord(const std::string& recordName) override ;

    virtual bool SetRecordInt(const std::string& recordName, const int row, const int col, const INT64 nValue) override ;
    virtual bool SetRecordFloat(const std::string& recordName, const int row, const int col, const double dwValue) override ;
    virtual bool SetRecordString(const std::string& recordName, const int row, const int col, const std::string& value) override ;
    virtual bool SetRecordObject(const std::string& recordName, const int row, const int col, const Guid& obj) override ;
	virtual bool SetRecordVector2(const std::string& recordName, const int row, const int col, const Vector2& value) override ;
	virtual bool SetRecordVector3(const std::string& recordName, const int row, const int col, const Vector3& value) override ;

    virtual bool SetRecordInt(const std::string& recordName, const int row, const std::string& colTag, const INT64 value) override ;
    virtual bool SetRecordFloat(const std::string& recordName, const int row, const std::string& colTag, const double value) override ;
    virtual bool SetRecordString(const std::string& recordName, const int row, const std::string& colTag, const std::string& value) override ;
    virtual bool SetRecordObject(const std::string& recordName, const int row, const std::string& colTag, const Guid& value) override ;
	virtual bool SetRecordVector2(const std::string& recordName, const int row, const std::string& colTag, const Vector2& value) override ;
	virtual bool SetRecordVector3(const std::string& recordName, const int row, const std::string& colTag, const Vector3& value) override ;


    virtual INT64 GetRecordInt(const std::string& recordName, const int row, const int col) override ;
    virtual double GetRecordFloat(const std::string& recordName, const int row, const int col) override ;
    virtual const std::string& GetRecordString(const std::string& recordName, const int row, const int col) override ;
    virtual const Guid& GetRecordObject(const std::string& recordName, const int row, const int col) override ;
	virtual const Vector2& GetRecordVector2(const std::string& recordName, const int row, const int col) override ;
	virtual const Vector3& GetRecordVector3(const std::string& recordName, const int row, const int col) override ;

    virtual INT64 GetRecordInt(const std::string& recordName, const int row, const std::string& colTag) override ;
    virtual double GetRecordFloat(const std::string& recordName, const int row, const std::string& colTag) override ;
    virtual const std::string& GetRecordString(const std::string& recordName, const int row, const std::string& colTag) override ;
    virtual const Guid& GetRecordObject(const std::string& recordName, const int row, const std::string& colTag) override ;
	virtual const Vector2& GetRecordVector2(const std::string& recordName, const int row, const std::string& colTag) override ;
	virtual const Vector3& GetRecordVector3(const std::string& recordName, const int row, const std::string& colTag) override ;

    virtual SQUICK_SHARE_PTR<IRecordManager> GetRecordManager() override ;
    virtual SQUICK_SHARE_PTR<IPropertyManager> GetPropertyManager() override ;

	virtual void SetRecordManager(SQUICK_SHARE_PTR<IRecordManager> xRecordManager) override ;
	virtual void SetPropertyManager(SQUICK_SHARE_PTR<IPropertyManager> xPropertyManager) override ;

protected:
    virtual bool AddRecordCallBack(const std::string& recordName, const RECORD_EVENT_FUNCTOR_PTR& cb) override ;

    virtual bool AddPropertyCallBack(const std::string& strCriticalName, const PROPERTY_EVENT_FUNCTOR_PTR& cb) override ;

private:
	Guid mSelf;
	Vector3 mPosition;
	CLASS_OBJECT_EVENT mObjectEventState;
    SQUICK_SHARE_PTR<IRecordManager> m_pRecordManager;
    SQUICK_SHARE_PTR<IPropertyManager> m_pPropertyManager;

};

#endif