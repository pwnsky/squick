

#ifndef SQUICK_SCHEDULE_MODULE_H
#define SQUICK_SCHEDULE_MODULE_H

#include <algorithm>
#include <iostream>
#include <set>
#include <squick/core/data_list.h>
#include <squick/core/date_time.h>
#include <squick/core/list.h>
#include <squick/core/map.h>
#include <squick/core/performance.h>
#include <squick/plugin/log/i_log_module.h>

#include "i_kernel_module.h"
#include "i_scene_module.h"
#include "i_schedule_module.h"

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
    int OnClassCommonEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var);

    int OnGroupCommonEvent(const Guid &self, const int scene, const int group, const int type, const DataList &arg);

  protected:
    MapEx<Guid, MapEx<std::string, ScheduleElement>> mObjectScheduleMap;

    std::multiset<TickElement> mScheduleMap;

    ILogModule *m_log_;
    IKernelModule *m_kernel_;
    ISceneModule *m_scene_;
};

#endif