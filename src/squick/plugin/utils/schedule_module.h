#pragma once

#include "i_schedule_module.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <squick/core/data_list.h>
#include <squick/core/date_time.h>
#include <squick/core/list.h>
#include <squick/core/map.h>
#include <squick/core/performance.h>
#include <squick/plugin/log/i_log_module.h>

#if PLATFORM != PLATFORM_WIN
#include "squick/core/exception.h"
#endif

class TickElement {
  public:
    bool operator<(const TickElement &b) const { return this->triggerTime < b.triggerTime; }

    std::string scheduleName;
    INT64 triggerTime;
    Guid self;
};

class ScheduleModule : public IScheduleModule {
  public:
    ScheduleModule(IPluginManager *p);

    virtual ~ScheduleModule();

    virtual bool Start();
    virtual bool Update();

    virtual bool AddSchedule(const Guid self, const std::string &scheduleName, const OBJECT_SCHEDULE_FUNCTOR_PTR &cb, const float time, const int count);
    virtual bool RemoveSchedule(const Guid self);
    virtual bool RemoveSchedule(const Guid self, const std::string &scheduleName);
    virtual bool ExistSchedule(const Guid self, const std::string &scheduleName);
    virtual std::shared_ptr<ScheduleElement> GetSchedule(const Guid self, const std::string &scheduleName);

  protected:
    MapEx<Guid, MapEx<std::string, ScheduleElement>> mObjectScheduleMap;

    std::multiset<TickElement> mScheduleMap;

    ILogModule *m_log_;
};