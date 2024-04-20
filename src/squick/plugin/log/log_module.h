#pragma once

#include "i_log_module.h"
#include <squick/core/performance.h>

class LogModule : public ILogModule {
  public:
    LogModule(IPluginManager *p);
    virtual ~LogModule() {}

    virtual bool Awake();
    virtual bool Start();
    virtual bool Destroy();

    virtual bool BeforeDestroy();
    virtual bool AfterStart();

    virtual bool Update();
    virtual void LogStack();

    virtual el::Logger *GetLogger() override;

  protected:
    static bool CheckLogFileExist(const char *filename);
    static void rolloutHandler(const char *filename, std::size_t size);

    std::string GetConfigPath(const std::string &fileName);

  private:
    std::string mstrLocalStream;
    static unsigned int idx;
    uint64_t mnLogCountTotal;
    std::list<Performance> mxPerformanceList;
    el::Logger *logger_ = nullptr;
};