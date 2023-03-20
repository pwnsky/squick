

#ifndef SQUICK_INTF_PROPERTYMANAGER_H
#define SQUICK_INTF_PROPERTYMANAGER_H

#include "i_property.h"
#include "map.h"
#include "platform.h"

class _SquickExport IPropertyManager : public MapEx<std::string, IProperty> {
  public:
    virtual ~IPropertyManager() {}

    virtual bool RegisterCallback(const std::string &propertyName, const PROPERTY_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual SQUICK_SHARE_PTR<IProperty> AddProperty(const Guid &self, SQUICK_SHARE_PTR<IProperty> pProperty) = 0;

    virtual SQUICK_SHARE_PTR<IProperty> AddProperty(const Guid &self, const std::string &propertyName, const DATA_TYPE varType) = 0;

    virtual const Guid &Self() = 0;

    virtual std::string ToString() = 0;
    //////////////////////////////////////////////////////////////////////////
    virtual bool SetProperty(const std::string &propertyName, const SquickData &TData) = 0;

    virtual bool SetPropertyInt(const std::string &propertyName, const INT64 nValue) = 0;
    virtual bool SetPropertyFloat(const std::string &propertyName, const double dwValue) = 0;
    virtual bool SetPropertyString(const std::string &propertyName, const std::string &value) = 0;
    virtual bool SetPropertyObject(const std::string &propertyName, const Guid &obj) = 0;
    virtual bool SetPropertyVector2(const std::string &propertyName, const Vector2 &value) = 0;
    virtual bool SetPropertyVector3(const std::string &propertyName, const Vector3 &value) = 0;

    virtual INT64 GetPropertyInt(const std::string &propertyName) = 0;
    virtual int GetPropertyInt32(const std::string &propertyName) = 0;
    virtual double GetPropertyFloat(const std::string &propertyName) = 0;
    virtual const std::string &GetPropertyString(const std::string &propertyName) = 0;
    virtual const Guid &GetPropertyObject(const std::string &propertyName) = 0;
    virtual const Vector2 &GetPropertyVector2(const std::string &propertyName) = 0;
    virtual const Vector3 &GetPropertyVector3(const std::string &propertyName) = 0;

    //////////////////////////////////////////////////////////////////////////

  private:
};

#endif
