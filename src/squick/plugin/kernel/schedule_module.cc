
#include "schedule_module.h"

void ScheduleElement::DoHeartBeatEvent(INT64 nowTime) {
    if (mnRemainCount > 0) {
        mnRemainCount--;
    }

    mnTriggerTime = nowTime + (INT64)(mfIntervalTime * 1000);

#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
    SQUICK_CRASH_TRY
#endif
    OBJECT_SCHEDULE_FUNCTOR_PTR cb;
    bool bRet = this->mxObjectFunctor.First(cb);
    while (bRet) {

        cb.get()->operator()(self, mstrScheduleName, mfIntervalTime, mnRemainCount);

        bRet = this->mxObjectFunctor.Next(cb);
    }
#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
    SQUICK_CRASH_END
#endif
}

ScheduleModule::ScheduleModule(IPluginManager *p) {
    pPluginManager = p;
    m_bIsUpdate = true;
}

ScheduleModule::~ScheduleModule() { mObjectScheduleMap.ClearAll(); }

bool ScheduleModule::Start() {
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pSceneModule = pPluginManager->FindModule<ISceneModule>();

    m_pKernelModule->RegisterCommonClassEvent(this, &ScheduleModule::OnClassCommonEvent);
    m_pSceneModule->AddSceneGroupDestroyedCallBack(this, &ScheduleModule::OnGroupCommonEvent);

    return true;
}

bool ScheduleModule::Update() {
    Performance performanceObject;
    INT64 nowTime = SquickGetTimeMS();

    static std::vector<TickElement> elements;
    elements.clear();

    for (auto it = mScheduleMap.begin(); it != mScheduleMap.end();) {
        if (nowTime >= it->triggerTime) {
            // std::cout << nowTime << it->scheduleName << ">>>>>" << it->triggerTime << std::endl;

            auto objectMap = mObjectScheduleMap.GetElement(it->self);
            if (objectMap) {
                auto scheduleElement = objectMap->GetElement(it->scheduleName);
                if (scheduleElement) {
                    if (scheduleElement->mnRemainCount == 1) {
                        objectMap->RemoveElement(it->scheduleName);
                    }
                    scheduleElement->DoHeartBeatEvent(nowTime);

                    if (scheduleElement->mnRemainCount != 0) {
                        TickElement element;
                        element.scheduleName = scheduleElement->mstrScheduleName;
                        element.triggerTime = scheduleElement->mnTriggerTime;
                        element.self = scheduleElement->self;

                        elements.push_back(element);
                    }
                }
            }

            it = mScheduleMap.erase(it);
        } else {
            break;
        }
    }

    for (auto &item : elements) {
        mScheduleMap.insert(item);
    }

    if (performanceObject.CheckTimePoint(1)) {
        std::ostringstream os;
        os << "---------------module schedule performance problem ";
        os << performanceObject.TimeScope();
        os << "---------- ";
        m_pLogModule->LogWarning(Guid(), os, __FUNCTION__, __LINE__);
    }

    return true;
}

bool ScheduleModule::AddSchedule(const Guid self, const std::string &scheduleName, const OBJECT_SCHEDULE_FUNCTOR_PTR &cb, const float time, const int count) {
    auto objectMap = mObjectScheduleMap.GetElement(self);
    if (!objectMap) {
        objectMap = SQUICK_SHARE_PTR<MapEx<std::string, ScheduleElement>>(SQUICK_NEW MapEx<std::string, ScheduleElement>());
        mObjectScheduleMap.AddElement(self, objectMap);
    }

    auto scheduleObject = objectMap->GetElement(scheduleName);
    if (!scheduleObject) {
        scheduleObject = SQUICK_SHARE_PTR<ScheduleElement>(SQUICK_NEW ScheduleElement());
        scheduleObject->mstrScheduleName = scheduleName;
        scheduleObject->mfIntervalTime = time;
        scheduleObject->mnTriggerTime = SquickGetTimeMS() + (INT64)(time * 1000);
        scheduleObject->mnRemainCount = count;
        scheduleObject->self = self;

        scheduleObject->mxObjectFunctor.Add(cb);

        objectMap->AddElement(scheduleName, scheduleObject);

        TickElement tickElement;
        tickElement.scheduleName = scheduleObject->mstrScheduleName;
        tickElement.triggerTime = scheduleObject->mnTriggerTime;
        tickElement.self = scheduleObject->self;

        mScheduleMap.insert(tickElement);
    }

    return true;
}

bool ScheduleModule::RemoveSchedule(const Guid self) { return mObjectScheduleMap.RemoveElement(self); }

bool ScheduleModule::RemoveSchedule(const Guid self, const std::string &scheduleName) {
    auto objectMap = mObjectScheduleMap.GetElement(self);
    if (objectMap) {
        return objectMap->RemoveElement(scheduleName);
    }

    return false;
}

bool ScheduleModule::ExistSchedule(const Guid self, const std::string &scheduleName) {
    auto objectScheduleMap = mObjectScheduleMap.GetElement(self);
    if (NULL == objectScheduleMap) {
        return false;
    }

    return objectScheduleMap->ExistElement(scheduleName);
}

SQUICK_SHARE_PTR<ScheduleElement> ScheduleModule::GetSchedule(const Guid self, const std::string &scheduleName) {
    SQUICK_SHARE_PTR<MapEx<std::string, ScheduleElement>> xObjectScheduleMap = mObjectScheduleMap.GetElement(self);
    if (NULL == xObjectScheduleMap) {
        return nullptr;
    }

    return xObjectScheduleMap->GetElement(scheduleName);
}

int ScheduleModule::OnClassCommonEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var) {
    if (CLASS_OBJECT_EVENT::COE_DESTROY == classEvent) {
        this->RemoveSchedule(self);
    }

    return 0;
}

int ScheduleModule::OnGroupCommonEvent(const Guid &self, const int scene, const int group, const int type, const DataList &arg) {
    this->RemoveSchedule(Guid(scene, group));
    return 0;
}
