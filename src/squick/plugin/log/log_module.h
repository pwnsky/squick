
#ifndef SQUICK_LOG_MODULE_H
#define SQUICK_LOG_MODULE_H

#include "i_log_module.h"
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/core/performance.h>

class LogModule
    : public ILogModule
{
public:

    LogModule(IPluginManager* p);
    virtual ~LogModule() {}

    virtual bool Awake();
    virtual bool Start();
    virtual bool Destory();

    virtual bool BeforeDestory();
    virtual bool AfterStart();

    virtual bool Update();

    ///////////////////////////////////////////////////////////////////////
    virtual void LogStack();

    virtual bool LogRecord(const SQUICK_LOG_LEVEL nll, const Guid ident, const std::string& recordName, const std::string& strDesc = "", const char* func = "", int line = 0);
    virtual bool LogObject(const SQUICK_LOG_LEVEL nll, const Guid ident, const std::string& strDesc, const char* func = "", int line = 0);

    virtual bool LogDebug(const std::string& strLog, const char* func = "", int line = 0);
    virtual bool LogInfo(const std::string& strLog, const  char* func = "", int line = 0);
    virtual bool LogWarning(const std::string& strLog, const char* func = "", int line = 0);
    virtual bool LogError(const std::string& strLog, const char* func = "", int line = 0);
    virtual bool LogFatal(const std::string& strLog, const char* func = "", int line = 0);

    virtual bool LogDebug(const std::ostringstream& stream, const char* func = "", int line = 0);
    virtual bool LogInfo(const std::ostringstream& stream, const  char* func = "", int line = 0);
    virtual bool LogWarning(const std::ostringstream& stream, const char* func = "", int line = 0);
    virtual bool LogError(const std::ostringstream& stream, const char* func = "", int line = 0);
    virtual bool LogFatal(const std::ostringstream& stream, const char* func = "", int line = 0);

    virtual bool LogDebug(const Guid ident, const std::string& strLog, const char* func = "", int line = 0);
    virtual bool LogInfo(const Guid ident, const std::string& strLog, const  char* func = "", int line = 0);
    virtual bool LogWarning(const Guid ident, const std::string& strLog, const char* func = "", int line = 0);
    virtual bool LogError(const Guid ident, const std::string& strLog, const char* func = "", int line = 0);
    virtual bool LogFatal(const Guid ident, const std::string& strLog, const char* func = "", int line = 0);

    virtual bool LogDebug(const Guid ident, const std::ostringstream& stream, const char* func = "", int line = 0);
    virtual bool LogInfo(const Guid ident, const std::ostringstream& stream, const  char* func = "", int line = 0);
    virtual bool LogWarning(const Guid ident, const std::ostringstream& stream, const char* func = "", int line = 0);
    virtual bool LogError(const Guid ident, const std::ostringstream& stream, const char* func = "", int line = 0);
    virtual bool LogFatal(const Guid ident, const std::ostringstream& stream, const char* func = "", int line = 0);

    virtual bool LogDebugFunctionDump(const Guid ident, const int nMsg, const std::string& strArg, const char* func = "", const int line = 0);
    virtual bool ChangeLogLevel(const std::string& strLevel);
    
    virtual void SetHooker(LOG_HOOKER_FUNCTOR_PTR hooker);

protected:
    virtual bool Log(const SQUICK_LOG_LEVEL nll, const char* format, ...);

    static bool CheckLogFileExist(const char* filename);
    static void rolloutHandler(const char* filename, std::size_t size);

	std::string GetConfigPath(const std::string& fileName);

private:

	IKernelModule* m_pKernelModule;

    std::string mstrLocalStream;
    LOG_HOOKER_FUNCTOR_PTR mLogHooker;
    static unsigned int idx;
    uint64_t mnLogCountTotal;
	std::list<Performance> mxPerformanceList;
};

#endif
