
#ifndef SQUICK_PROPERTYMANAGER_H
#define SQUICK_PROPERTYMANAGER_H

#include "data_list.h"
#include "i_property_manager.h"
#include "map.h"
#include "platform.h"

class _SquickExport PropertyManager : public IPropertyManager {
  public:
    PropertyManager(const Guid &self) { mSelf = self; };

    virtual ~PropertyManager();

    virtual bool RegisterCallback(const std::string &propertyName, const PROPERTY_EVENT_FUNCTOR_PTR &cb);

    virtual SQUICK_SHARE_PTR<IProperty> AddProperty(const Guid &self, SQUICK_SHARE_PTR<IProperty> pProperty);

    virtual SQUICK_SHARE_PTR<IProperty> AddProperty(const Guid &self, const std::string &propertyName, const DATA_TYPE varType);

    virtual const Guid &Self();

    virtual std::string ToString();

    virtual bool SetProperty(const std::string &propertyName, const SquickData &TData);

    virtual bool SetPropertyInt(const std::string &propertyName, const INT64 nValue);
    virtual bool SetPropertyFloat(const std::string &propertyName, const double dwValue);
    virtual bool SetPropertyString(const std::string &propertyName, const std::string &value);
    virtual bool SetPropertyObject(const std::string &propertyName, const Guid &obj);
    virtual bool SetPropertyVector2(const std::string &propertyName, const Vector2 &value);
    virtual bool SetPropertyVector3(const std::string &propertyName, const Vector3 &value);

    virtual INT64 GetPropertyInt(const std::string &propertyName);
    virtual int GetPropertyInt32(const std::string &propertyName);
    virtual double GetPropertyFloat(const std::string &propertyName);
    virtual const std::string &GetPropertyString(const std::string &propertyName);
    virtual const Guid &GetPropertyObject(const std::string &propertyName);
    virtual const Vector2 &GetPropertyVector2(const std::string &propertyName);
    virtual const Vector3 &GetPropertyVector3(const std::string &propertyName);

  private:
    Guid mSelf;
    std::map<std::string, int> mxPropertyIndexMap;
};

#endif
