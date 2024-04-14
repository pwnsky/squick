
#pragma once

#include <squick/core/i_module.h>
#include <squick/core/easylogging++.h>

#ifdef SQUICK_DEV

    #define LOG_INFO(format, ...)   m_log_->GetLogger()->info  ("[%v][%v:%v] [%v] " format, pm_->GetAppName(), __FUNCTION__, __LINE__, this->name_, __VA_ARGS__)
    #define LOG_ERROR(format, ...)  m_log_->GetLogger()->error ("[%v][%v:%v] [%v] " format, pm_->GetAppName(), __FUNCTION__, __LINE__, this->name_, __VA_ARGS__)
    #define LOG_WARN(format, ...)   m_log_->GetLogger()->warn  ("[%v][%v:%v] [%v] " format, pm_->GetAppName(), __FUNCTION__, __LINE__, this->name_, __VA_ARGS__)
    #define LOG_FATAL(format, ...)  m_log_->GetLogger()->fatal ("[%v][%v:%v] [%v] " format, pm_->GetAppName(), __FUNCTION__, __LINE__, this->name_, __VA_ARGS__)

#else
    #define LOG_INFO(format, ...)   m_log_->GetLogger()->info  ("[%v] " format, this->name_, __VA_ARGS__)
    #define LOG_ERROR(format, ...)  m_log_->GetLogger()->error ("[%v] " format, this->name_, __VA_ARGS__)
    #define LOG_WARN(format, ...)   m_log_->GetLogger()->warn  ("[%v] " format, this->name_, __VA_ARGS__)
    #define LOG_FATAL(format, ...)  m_log_->GetLogger()->fatal ("[%v] " format, this->name_, __VA_ARGS__)
#endif

class ILogModule : public IModule {

  public:
    enum SQUICK_LOG_LEVEL {
        NLL_DEBUG_NORMAL,
        NLL_INFO_NORMAL,
        NLL_WARING_NORMAL,
        NLL_ERROR_NORMAL,
        NLL_FATAL_NORMAL,
    };

    typedef std::function<void(const ILogModule::SQUICK_LOG_LEVEL, const std::string &)> LOG_HOOKER_FUNCTOR;
    typedef std::shared_ptr<LOG_HOOKER_FUNCTOR> LOG_HOOKER_FUNCTOR_PTR;

    virtual bool LogDebug(const std::string &strLog, const char *func = "", int line = 0) = 0;
    virtual bool LogInfo(const std::string &strLog, const char *func = "", int line = 0) = 0;
    virtual bool LogWarning(const std::string &strLog, const char *func = "", int line = 0) = 0;
    virtual bool LogError(const std::string &strLog, const char *func = "", int line = 0) = 0;
    virtual bool LogFatal(const std::string &strLog, const char *func = "", int line = 0) = 0;

    virtual bool LogDebug(const std::ostringstream &stream, const char *func = "", int line = 0) = 0;
    virtual bool LogInfo(const std::ostringstream &stream, const char *func = "", int line = 0) = 0;
    virtual bool LogWarning(const std::ostringstream &stream, const char *func = "", int line = 0) = 0;
    virtual bool LogError(const std::ostringstream &stream, const char *func = "", int line = 0) = 0;
    virtual bool LogFatal(const std::ostringstream &stream, const char *func = "", int line = 0) = 0;

    virtual bool LogDebug(const Guid ident, const std::string &strLog, const char *func = "", int line = 0) = 0;
    virtual bool LogInfo(const Guid ident, const std::string &strLog, const char *func = "", int line = 0) = 0;
    virtual bool LogWarning(const Guid ident, const std::string &strLog, const char *func = "", int line = 0) = 0;
    virtual bool LogError(const Guid ident, const std::string &strLog, const char *func = "", int line = 0) = 0;
    virtual bool LogFatal(const Guid ident, const std::string &strLog, const char *func = "", int line = 0) = 0;

    virtual bool LogDebug(const Guid ident, const std::ostringstream &stream, const char *func = "", int line = 0) = 0;
    virtual bool LogInfo(const Guid ident, const std::ostringstream &stream, const char *func = "", int line = 0) = 0;
    virtual bool LogWarning(const Guid ident, const std::ostringstream &stream, const char *func = "", int line = 0) = 0;
    virtual bool LogError(const Guid ident, const std::ostringstream &stream, const char *func = "", int line = 0) = 0;
    virtual bool LogFatal(const Guid ident, const std::ostringstream &stream, const char *func = "", int line = 0) = 0;

    template <typename BaseType> void SetHooker(BaseType *pBase, void (BaseType::*handler)(const ILogModule::SQUICK_LOG_LEVEL, const std::string &)) {
        auto functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2);
        LOG_HOOKER_FUNCTOR_PTR functorPtr(new LOG_HOOKER_FUNCTOR(functor));
        return SetHooker(functorPtr);
    }

    virtual el::Logger* GetLogger() = 0;

  protected:
    virtual void SetHooker(LOG_HOOKER_FUNCTOR_PTR hooker) = 0;
};