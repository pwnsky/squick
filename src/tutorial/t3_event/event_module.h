#pragma once
#include <squick/plugin/config/export.h>
#include <squick/plugin/world/export.h>

namespace tutorial {
class IEventModule : public IModule {};

class EventModule : public IEventModule {
  public:
    EventModule(IPluginManager *p) {
        pm_ = p;
    }

    virtual bool Start();
    virtual bool AfterStart();

  protected:
    int OnEvent(const Guid &self, const int event, const DataList &arg);
    int OnClassCallBackEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT event, const DataList &arg);
    int OnPropertyCallBackEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVarList, const SquickData &newVarList,
                                const int64_t reason);
    int OnPropertyStrCallBackEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVarList, const SquickData &newVarList,
                                   const int64_t reason);
    int OnHeartBeat(const Guid &self, const std::string &heartBeat, const float time, const int count);

  protected:
    int64_t mLastTime;

  protected:
    IKernelModule *m_kernel_;
    IElementModule *m_element_;
    ::IEventModule *m_event_;
    IScheduleModule *m_schedule_;
};
}