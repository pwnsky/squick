#pragma once

#include "i_world_module.h"

#include "i_data_tail_module.h"
#include <squick/core/base.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>

class DataTailModule : public IDataTailModule {
  public:
    DataTailModule(IPluginManager *p) { pm_ = p; }
    virtual ~DataTailModule(){};

    virtual bool Start();
    virtual bool Destroy();
    virtual bool Update();
    virtual bool AfterStart();

    virtual void LogObjectData(const Guid &self);
    virtual void StartTrail(const Guid &self);

  protected:
    void PrintStackTrace();

    int TrailObjectData(const Guid &self);

    int OnClassObjectEvent(const Guid &self, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &var);

    int OnObjectPropertyEvent(const Guid &self, const std::string &propertyName, const SquickData &oldVar, const SquickData &newVar, const INT64 reason);

    int OnObjectRecordEvent(const Guid &self, const RECORD_EVENT_DATA &eventData, const SquickData &oldVar, const SquickData &newVar);

  private:
    IKernelModule *m_kernel_;
    IElementModule *m_element_;
    IClassModule *m_class_;
    ILogModule *m_log_;
};
