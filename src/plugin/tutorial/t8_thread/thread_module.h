#pragma
#include <plugin/core/thread/export.h>
#include <thread>

class IThreadModule : public IModule {};
class ThreadModule : public IThreadModule {
  public:
    ThreadModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }
    virtual bool Start();
    virtual bool AfterStart();

  protected:
    IThreadPoolModule *m_thread_pool_;
};
