

#ifndef SQUICK_DATA_TRAIL_MODULE_H
#define SQUICK_DATA_TRAIL_MODULE_H

#include "i_kernel_module.h"

#include "i_data_tail_module.h"
#include <squick/core/i_plugin_manager.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/log/i_log_module.h>

class DataTailModule : public IDataTailModule {
  public:
    DataTailModule(IPluginManager *p) { pm_ = p; }
    virtual ~DataTailModule(){};

    virtual bool Start();
    virtual bool Destory();
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

#endif
