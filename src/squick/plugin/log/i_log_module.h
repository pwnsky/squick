

#ifndef SQUICK_INTF_LOG_MODULE_H
#define SQUICK_INTF_LOG_MODULE_H

#include <squick/core/i_module.h>

class ILogModule
    : public IModule
{

public:

    enum SQUICK_LOG_LEVEL
    {
        NLL_DEBUG_NORMAL,
		NLL_INFO_NORMAL,
        NLL_WARING_NORMAL,
        NLL_ERROR_NORMAL,
        NLL_FATAL_NORMAL,
    };

    typedef std::function<void(const ILogModule::SQUICK_LOG_LEVEL, const std::string&)> LOG_HOOKER_FUNCTOR;
    typedef SQUICK_SHARE_PTR<LOG_HOOKER_FUNCTOR> LOG_HOOKER_FUNCTOR_PTR;

    virtual bool LogObject(const SQUICK_LOG_LEVEL nll, const Guid ident, const std::string& strDesc, const char* func = "", int line = 0) = 0;
    virtual bool LogRecord(const SQUICK_LOG_LEVEL nll, const Guid ident, const std::string& recordName, const std::string& strDesc = "", const char* func = "", int line = 0) = 0;

    virtual bool LogDebug(const std::string& strLog, const char* func = "", int line = 0) = 0;
    virtual bool LogInfo(const std::string& strLog, const  char* func = "", int line = 0) = 0;
    virtual bool LogWarning(const std::string& strLog, const char* func = "", int line = 0) = 0;
    virtual bool LogError(const std::string& strLog, const char* func = "", int line = 0) = 0;
    virtual bool LogFatal(const std::string& strLog, const char* func = "", int line = 0) = 0;

    virtual bool LogDebug(const std::ostringstream& stream, const char* func = "", int line = 0) = 0;
    virtual bool LogInfo(const std::ostringstream& stream, const  char* func = "", int line = 0) = 0;
    virtual bool LogWarning(const std::ostringstream& stream, const char* func = "", int line = 0) = 0;
    virtual bool LogError(const std::ostringstream& stream, const char* func = "", int line = 0) = 0;
    virtual bool LogFatal(const std::ostringstream& stream, const char* func = "", int line = 0) = 0;

    virtual bool LogDebug(const Guid ident, const std::string& strLog, const char* func = "", int line = 0) = 0;
    virtual bool LogInfo(const Guid ident, const std::string& strLog, const  char* func = "", int line = 0) = 0;
    virtual bool LogWarning(const Guid ident, const std::string& strLog, const char* func = "", int line = 0) = 0;
    virtual bool LogError(const Guid ident, const std::string& strLog, const char* func = "", int line = 0) = 0;
    virtual bool LogFatal(const Guid ident, const std::string& strLog, const char* func = "", int line = 0) = 0;

    virtual bool LogDebug(const Guid ident, const std::ostringstream& stream, const char* func = "", int line = 0) = 0;
    virtual bool LogInfo(const Guid ident, const std::ostringstream& stream, const  char* func = "", int line = 0) = 0;
    virtual bool LogWarning(const Guid ident, const std::ostringstream& stream, const char* func = "", int line = 0) = 0;
    virtual bool LogError(const Guid ident, const std::ostringstream& stream, const char* func = "", int line = 0) = 0;
    virtual bool LogFatal(const Guid ident, const std::ostringstream& stream, const char* func = "", int line = 0) = 0;

    template<typename BaseType>
    void SetHooker(BaseType* pBase, void (BaseType::*handler)(const ILogModule::SQUICK_LOG_LEVEL, const std::string&))
    {
        auto functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2);
        LOG_HOOKER_FUNCTOR_PTR functorPtr(SQUICK_NEW LOG_HOOKER_FUNCTOR(functor));
        return SetHooker(functorPtr);
    }

protected:
    virtual void SetHooker(LOG_HOOKER_FUNCTOR_PTR hooker) = 0;
};

#endif