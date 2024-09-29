

#include "log_module.h"

#include "plugin.h"
#include "core/exception.h"
#include <core/termcolor.h>
#include <stdarg.h>

// Ref https://github.com/abumq/easyloggingpp;
// INITIALIZE_EASYLOGGINGPP

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

bool LogModule::Start() { return true; }

bool LogModule::Destroy() {
    el::Helpers::uninstallPreRollOutCallback();
    logger_->flush();
    return true;
}

bool LogModule::BeforeDestroy() { return true; }

bool LogModule::AfterStart() { return true; }

bool LogModule::Update() { return true; }

void LogModule::LogStack() {
#if PLATFORM != PLATFORM_WIN
    Exception::CrashHandler(0);
#endif
}

el::Logger *LogModule::GetLogger() { return logger_; }
