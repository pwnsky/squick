

#ifndef SQUICK_INTF_SCHEDULE_MODULE_H
#define SQUICK_INTF_SCHEDULE_MODULE_H

#include <squick/core/data_list.h>
#include <squick/core/date_time.h>
#include <squick/core/i_module.h>
#include <squick/core/list.h>
#include <squick/core/map.h>
#include <string>
// int ScheduleFunction(const Guid& self, const std::string& name, const float time, const int count);
typedef std::function<int(const Guid &, const std::string &, const float, const int)> OBJECT_SCHEDULE_FUNCTOR;
typedef std::shared_ptr<OBJECT_SCHEDULE_FUNCTOR> OBJECT_SCHEDULE_FUNCTOR_PTR; // HEART

class ScheduleElement {
  public:
    ScheduleElement() {
        mstrScheduleName = "";
        mfIntervalTime = 0.0f;
        mnTriggerTime = 0;
        mnRemainCount = -1;

        static INT64 elementID = 0;
        id = elementID++;
    };

    virtual ~ScheduleElement() {}

    void DoHeartBeatEvent(INT64 nowTime);

    std::string mstrScheduleName;
    float mfIntervalTime;
    INT64 mnTriggerTime;
    int mnRemainCount;
    INT64 id;

    Guid self;

    List<OBJECT_SCHEDULE_FUNCTOR_PTR> mxObjectFunctor;
};

class IScheduleModule : public IModule {
  public:
    virtual ~IScheduleModule() {}

    /// for object
    virtual bool RemoveSchedule(const Guid self) = 0;
    virtual bool RemoveSchedule(const Guid self, const std::string &scheduleName) = 0;
    virtual bool ExistSchedule(const Guid self, const std::string &scheduleName) = 0;
    virtual std::shared_ptr<ScheduleElement> GetSchedule(const Guid self, const std::string &scheduleName) = 0;

    template <typename BaseType>
    bool AddSchedule(const Guid self, const std::string &scheduleName, BaseType *pBase,
                     int (BaseType::*handler)(const Guid &self, const std::string &scheduleName, const float time, const int count), const float fIntervalTime,
                     const int count) {
        OBJECT_SCHEDULE_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        OBJECT_SCHEDULE_FUNCTOR_PTR functorPtr(new OBJECT_SCHEDULE_FUNCTOR(functor));
        return AddSchedule(self, scheduleName, functorPtr, fIntervalTime, count);
    }

  protected:
    virtual bool AddSchedule(const Guid self, const std::string &scheduleName, const OBJECT_SCHEDULE_FUNCTOR_PTR &cb, const float time, const int count) = 0;
};

#endif
