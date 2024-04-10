
#include "event_module.h"

bool EventModule::Start() {
    return true;
}

bool EventModule::AfterStart() { return true; }

bool EventModule::BeforeDestroy() {
    mModuleEventInfoMapEx.ClearAll();
    mObjectEventInfoMapEx.ClearAll();

    return true;
}

bool EventModule::Destroy() { return true; }

bool EventModule::Update() {
    // remove
    if (mModuleRemoveListEx.Count() > 0) {
        int eventID = 0;
        bool bRet = mModuleRemoveListEx.First(eventID);
        while (bRet) {
            mModuleEventInfoMapEx.RemoveElement(eventID);

            bRet = mModuleRemoveListEx.Next(eventID);
        }
    }

    //////
    if (mObjectRemoveListEx.Count() > 0) {
        Guid xObject;
        bool bRet = mObjectRemoveListEx.First(xObject);
        while (bRet) {
            mObjectEventInfoMapEx.RemoveElement(xObject);

            bRet = mObjectRemoveListEx.Next(xObject);
        }
    }

    return true;
}

bool EventModule::DoEvent(const int eventID, const DataList &valueList) {
    bool bRet = false;

    auto xEventListPtr = mModuleEventInfoMapEx.GetElement(eventID);
    if (xEventListPtr) {
        MODULE_EVENT_FUNCTOR pFunPtr;
        bool bRet = xEventListPtr->First(pFunPtr);
        while (bRet) {
            pFunPtr.operator()(eventID, valueList);

            bRet = xEventListPtr->Next(pFunPtr);
        }

        bRet = true;
    }

    return bRet;
}

bool EventModule::ExistEventCallBack(const int eventID) { return mModuleEventInfoMapEx.ExistElement(eventID); }

bool EventModule::RemoveEventCallBack(const int eventID) { return mModuleEventInfoMapEx.RemoveElement(eventID); }

bool EventModule::DoEvent(const Guid self, const int eventID, const DataList &valueList) {
    {
        // for common event call back
        OBJECT_EVENT_FUNCTOR pFunPtr;
        bool bFunRet = mCommonEventInfoMapEx.First(pFunPtr);
        while (bFunRet) {
            pFunPtr.operator()(self, eventID, valueList);

            bFunRet = mCommonEventInfoMapEx.Next(pFunPtr);
        }
    }

    bool bRet = false;

    auto xEventMapPtr = mObjectEventInfoMapEx.GetElement(self);
    if (!xEventMapPtr) {
        return bRet;
    }

    auto xEventListPtr = xEventMapPtr->GetElement(eventID);
    if (!xEventListPtr) {
        return bRet;
    }

    OBJECT_EVENT_FUNCTOR pFunPtr;
    bool bFunRet = xEventListPtr->First(pFunPtr);
    while (bFunRet) {
        pFunPtr.operator()(self, eventID, valueList);

        bFunRet = xEventListPtr->Next(pFunPtr);
    }

    return bRet;
}

bool EventModule::ExistEventCallBack(const Guid self, const int eventID) {
    auto xEventMapPtr = mObjectEventInfoMapEx.GetElement(self);
    if (!xEventMapPtr) {
        return false;
    }

    return xEventMapPtr->ExistElement(eventID);
}

bool EventModule::RemoveEventCallBack(const Guid self, const int eventID) {
    auto xEventMapPtr = mObjectEventInfoMapEx.GetElement(self);
    if (!xEventMapPtr) {
        return false;
    }

    return xEventMapPtr->RemoveElement(eventID);
}

bool EventModule::RemoveEventCallBack(const Guid self) { return mObjectEventInfoMapEx.RemoveElement(self); }

bool EventModule::AddEventCallBack(const int eventID, const MODULE_EVENT_FUNCTOR cb) {
    auto xEventListPtr = mModuleEventInfoMapEx.GetElement(eventID);
    if (!xEventListPtr) {
        xEventListPtr = std::shared_ptr<List<MODULE_EVENT_FUNCTOR>>(new List<MODULE_EVENT_FUNCTOR>());
        mModuleEventInfoMapEx.AddElement(eventID, xEventListPtr);
    }

    xEventListPtr->Add(cb);

    return false;
}

bool EventModule::AddEventCallBack(const Guid self, const int eventID, const OBJECT_EVENT_FUNCTOR cb) {

    auto xEventMapPtr = mObjectEventInfoMapEx.GetElement(self);
    if (!xEventMapPtr) {
        xEventMapPtr = std::shared_ptr<MapEx<int, List<OBJECT_EVENT_FUNCTOR>>>(new MapEx<int, List<OBJECT_EVENT_FUNCTOR>>());
        mObjectEventInfoMapEx.AddElement(self, xEventMapPtr);
    }

    auto xEventListPtr = xEventMapPtr->GetElement(eventID);
    if (!xEventListPtr) {
        xEventListPtr = std::shared_ptr<List<OBJECT_EVENT_FUNCTOR>>(new List<OBJECT_EVENT_FUNCTOR>());
        xEventMapPtr->AddElement(eventID, xEventListPtr);
    }

    xEventListPtr->Add(cb);

    return true;
}

bool EventModule::AddCommonEventCallBack(const OBJECT_EVENT_FUNCTOR cb) {
    mCommonEventInfoMapEx.Add(cb);
    return true;
}
