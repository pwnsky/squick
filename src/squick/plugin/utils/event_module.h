#pragma once

#include "i_event_module.h"
// #include "i_kernel_module.h"
#include <iostream>
#include <squick/core/guid.h>
#include <squick/core/i_object.h>

class EventModule : public IEventModule {
  public:
    EventModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }

    virtual ~EventModule() {}

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool BeforeDestroy();
    virtual bool Destroy();
    virtual bool Update();

    virtual bool DoEvent(const int eventID, const DataList &valueList);

    virtual bool ExistEventCallBack(const int eventID);

    virtual bool RemoveEventCallBack(const int eventID);

    //////////////////////////////////////////////////////////
    virtual bool DoEvent(const Guid self, const int eventID, const DataList &valueList);

    virtual bool ExistEventCallBack(const Guid self, const int eventID);

    virtual bool RemoveEventCallBack(const Guid self, const int eventID);
    virtual bool RemoveEventCallBack(const Guid self);

  protected:
    virtual bool AddEventCallBack(const int eventID, const MODULE_EVENT_FUNCTOR cb);
    virtual bool AddEventCallBack(const Guid self, const int eventID, const OBJECT_EVENT_FUNCTOR cb);
    virtual bool AddCommonEventCallBack(const OBJECT_EVENT_FUNCTOR cb);

  private:
    // IWorldModule *m_world_;

  private:
    // for module
    List<int> mModuleRemoveListEx;
    MapEx<int, List<MODULE_EVENT_FUNCTOR>> mModuleEventInfoMapEx;

    // for object
    List<Guid> mObjectRemoveListEx;
    MapEx<Guid, MapEx<int, List<OBJECT_EVENT_FUNCTOR>>> mObjectEventInfoMapEx;

    // for common event
    List<OBJECT_EVENT_FUNCTOR> mCommonEventInfoMapEx;
};