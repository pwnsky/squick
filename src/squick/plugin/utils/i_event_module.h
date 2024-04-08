#pragma once

#include <iostream>
#include <squick/core/i_module.h>

class IEventModule : public IModule {
  public:
  protected:
    typedef std::function<int(const Guid &, const int, const DataList &)> OBJECT_EVENT_FUNCTOR;
    typedef std::function<int(const int, const DataList &)> MODULE_EVENT_FUNCTOR;

  public:
    // only be used in module
    virtual bool DoEvent(const int eventID, const DataList &valueList) = 0;

    virtual bool ExistEventCallBack(const int eventID) = 0;

    virtual bool RemoveEventCallBack(const int eventID) = 0;

    template <typename BaseType> bool AddEventCallBack(const int eventID, BaseType *pBase, int (BaseType::*handler)(const int, const DataList &)) {
        MODULE_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2);
        return AddEventCallBack(eventID, functor);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // can be used for object
    virtual bool DoEvent(const Guid self, const int eventID, const DataList &valueList) = 0;

    virtual bool ExistEventCallBack(const Guid self, const int eventID) = 0;

    virtual bool RemoveEventCallBack(const Guid self, const int eventID) = 0;
    virtual bool RemoveEventCallBack(const Guid self) = 0;

    template <typename BaseType>
    bool AddEventCallBack(const Guid &self, const int eventID, BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const DataList &)) {
        OBJECT_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        return AddEventCallBack(self, eventID, functor);
    }

    // can be used for common event
    template <typename BaseType> bool AddCommonEventCallBack(BaseType *pBase, int (BaseType::*handler)(const Guid &, const int, const DataList &)) {
        OBJECT_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        return AddCommonEventCallBack(functor);
    }

  protected:
    virtual bool AddEventCallBack(const int eventID, const MODULE_EVENT_FUNCTOR cb) = 0;
    virtual bool AddEventCallBack(const Guid self, const int eventID, const OBJECT_EVENT_FUNCTOR cb) = 0;
    virtual bool AddCommonEventCallBack(const OBJECT_EVENT_FUNCTOR cb) = 0;
};