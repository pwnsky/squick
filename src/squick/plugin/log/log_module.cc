

#include "log_module.h"

#include "plugin.h"
#include "squick/core/exception.h"
#include <squick/core/termcolor.h>
#include <stdarg.h>

// Ref https://github.com/abumq/easyloggingpp;
//INITIALIZE_EASYLOGGINGPP

unsigned int LogModule::idx = 0;

bool LogModule::CheckLogFileExist(const char *filename) {
    std::stringstream stream;
    stream << filename << "." << ++idx;
    std::fstream file;
    file.open(stream.str(), std::ios::in);
    if (file) {
        return CheckLogFileExist(filename);
    }

    return false;
}

void LogModule::rolloutHandler(const char *filename, std::size_t size) {
    std::stringstream stream;
    if (!CheckLogFileExist(filename)) {
        stream << filename << "." << idx;
        rename(filename, stream.str().c_str());
    }
}

// 获取日志配置文件全路径
std::string LogModule::GetConfigPath(const std::string &fileName) {
    std::string strAppLogName;
    strAppLogName = pm_->GetWorkPath() + "/config/log/" + fileName + ".conf";

    return strAppLogName;
}

LogModule::LogModule(IPluginManager *p) {
    pm_ = p;

    el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
    el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
}

bool LogModule::Awake() {
    mnLogCountTotal = 0;
    
    string conf_path = pm_->GetArg("logconf=", "");
    if (conf_path.empty()) {
        conf_path = GetConfigPath(pm_->GetArg("type=", "default"));
    }

    el::Configurations conf(conf_path);
    el::Configuration *pConfiguration = conf.get(el::Level::Debug, el::ConfigurationType::Filename);
    if (pConfiguration == nullptr) {
        // Just make sure open correctly
        SQUICK_PRINT("Warnning: Use default log config, config/log/default.conf");
        conf = el::Configurations(GetConfigPath("default"));
    }

    int open_log = pm_->GetArg("logshow=", 1);
    if (open_log) {
        conf.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
    } else {
        conf.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
    }
    
    el::Helpers::installPreRollOutCallback(rolloutHandler);
    logger_ = el::Loggers::getLogger(pm_->GetAppName());
    logger_->configure(conf);

    return true;
}

bool LogModule::Start() {

    return true;
}

bool LogModule::Destroy() {
    el::Helpers::uninstallPreRollOutCallback();
    logger_->flush();
    return true;
}

bool LogModule::BeforeDestroy() { return true; }

bool LogModule::AfterStart() { return true; }

bool LogModule::Update() { return true; }

bool LogModule::Log(const SQUICK_LOG_LEVEL nll, const char *format, ...) {
    mnLogCountTotal++;

    char szBuffer[1024 * 10] = {0};

    va_list args;
    va_start(args, format);
    vsnprintf(szBuffer, sizeof(szBuffer) - 1, format, args);
    va_end(args);

    mstrLocalStream.clear();
    mstrLocalStream.append(this->pm_->GetAppName());
    mstrLocalStream.append(" | ");
    mstrLocalStream.append(std::to_string(pm_->GetAppID()));
    mstrLocalStream.append(" | ");
    mstrLocalStream.append(std::to_string(mnLogCountTotal));
    mstrLocalStream.append(" | ");
    mstrLocalStream.append(szBuffer);

    if (mLogHooker) {
        mLogHooker.get()->operator()(nll, mstrLocalStream);
    }
    switch (nll) {
    case ILogModule::NLL_DEBUG_NORMAL: {
        logger_->debug("%v", mstrLocalStream.c_str());
    } break;
    case ILogModule::NLL_INFO_NORMAL: {
        logger_->info("%v", mstrLocalStream.c_str());
    } break;
    case ILogModule::NLL_WARING_NORMAL: {
        std::cout << termcolor::yellow;
        logger_->warn("%v", mstrLocalStream.c_str());
        std::cout << termcolor::reset;
    } break;
    case ILogModule::NLL_ERROR_NORMAL: {
        std::cout << termcolor::red;
        logger_->error("%v", mstrLocalStream.c_str());
        std::cout << termcolor::reset;
    } break;
    case ILogModule::NLL_FATAL_NORMAL: {
        std::cout << termcolor::red;
        logger_->fatal("%v", mstrLocalStream.c_str());
        LogStack();
        std::cout << termcolor::reset;
    } break;
    default: {
        std::cout << termcolor::white;
        logger_->info("%v", mstrLocalStream.c_str());
        std::cout << termcolor::reset;
    } break;
    }

    return true;
}

void LogModule::LogStack() {
#if PLATFORM != PLATFORM_WIN
    Exception::CrashHandler(0);
#endif
}

bool LogModule::LogDebug(const std::string &strLog, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "%s %s %d", strLog.c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "%s", strLog.c_str());
    }

    return true;
}

bool LogModule::LogInfo(const std::string &strLog, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "%s %s %d", strLog.c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "%s", strLog.c_str());
    }

    return true;
}

bool LogModule::LogWarning(const std::string &strLog, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "%s %s %d", strLog.c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "%s", strLog.c_str());
    }

    return true;
}

bool LogModule::LogError(const std::string &strLog, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "%s %s %d", strLog.c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "%s", strLog.c_str());
    }

    return true;
}

bool LogModule::LogFatal(const std::string &strLog, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "%s %s %d", strLog.c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "%s", strLog.c_str());
    }

    return true;
}

bool LogModule::LogDebug(const std::ostringstream &stream, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "%s %s %d", stream.str().c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "%s", stream.str().c_str());
    }

    return true;
}

bool LogModule::LogInfo(const std::ostringstream &stream, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "%s %s %d", stream.str().c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "%s", stream.str().c_str());
    }

    return true;
}

bool LogModule::LogWarning(const std::ostringstream &stream, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "%s %s %d", stream.str().c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "%s", stream.str().c_str());
    }

    return true;
}

bool LogModule::LogError(const std::ostringstream &stream, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "%s %s %d", stream.str().c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "%s", stream.str().c_str());
    }

    return true;
}

bool LogModule::LogFatal(const std::ostringstream &stream, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "%s %s %d", stream.str().c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "%s", stream.str().c_str());
    }

    return true;
}

bool LogModule::LogDebug(const Guid ident, const std::string &strLog, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), strLog.c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), strLog.c_str());
    }

    return true;
}

bool LogModule::LogInfo(const Guid ident, const std::string &strLog, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), strLog.c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), strLog.c_str());
    }

    return true;
}

bool LogModule::LogWarning(const Guid ident, const std::string &strLog, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), strLog.c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), strLog.c_str());
    }

    return true;
}

bool LogModule::LogError(const Guid ident, const std::string &strLog, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), strLog.c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), strLog.c_str());
    }

    return true;
}

bool LogModule::LogFatal(const Guid ident, const std::string &strLog, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), strLog.c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), strLog.c_str());
    }

    return true;
}

bool LogModule::LogDebug(const Guid ident, const std::ostringstream &stream, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), stream.str().c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), stream.str().c_str());
    }

    return true;
}

bool LogModule::LogInfo(const Guid ident, const std::ostringstream &stream, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), stream.str().c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), stream.str().c_str());
    }

    return true;
}

bool LogModule::LogWarning(const Guid ident, const std::ostringstream &stream, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), stream.str().c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), stream.str().c_str());
    }

    return true;
}

bool LogModule::LogError(const Guid ident, const std::ostringstream &stream, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), stream.str().c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), stream.str().c_str());
    }

    return true;
}

bool LogModule::LogFatal(const Guid ident, const std::ostringstream &stream, const char *func, int line) {
    if (line > 0) {
        Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), stream.str().c_str(), func, line);
    } else {
        Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), stream.str().c_str());
    }

    return true;
}

void LogModule::SetHooker(LOG_HOOKER_FUNCTOR_PTR hooker) { mLogHooker = hooker; }

el::Logger* LogModule::GetLogger() { return logger_; }
