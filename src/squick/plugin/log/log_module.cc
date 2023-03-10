

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <stdarg.h>
#include "log_module.h"
#include "termcolor.h"
#include "easylogging++.h"
#include "plugin.h"
#include "squick/core/exception.h"

INITIALIZE_EASYLOGGINGPP

unsigned int LogModule::idx = 0;

bool LogModule::CheckLogFileExist(const char* filename)
{
    std::stringstream stream;
    stream << filename << "." << ++idx;
    std::fstream file;
    file.open(stream.str(), std::ios::in);
    if (file)
    {
        return CheckLogFileExist(filename);
    }

    return false;
}

void LogModule::rolloutHandler(const char* filename, std::size_t size)
{
    std::stringstream stream;
    if (!CheckLogFileExist(filename))
    {
        stream << filename << "." << idx;
        rename(filename, stream.str().c_str());
    }
}

// 获取日志配置文件全路径
std::string LogModule::GetConfigPath(const std::string & fileName)
{
	std::string strAppLogName;
    strAppLogName = pPluginManager->GetConfigPath() + "/config/log/" + fileName + ".conf";

	return strAppLogName;
}

LogModule::LogModule(IPluginManager* p)
{
    pPluginManager = p;

	el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
	el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
}

bool LogModule::Awake()
{
	mnLogCountTotal = 0;

	std::string strLogConfigName = pPluginManager->GetLogConfigName();
	if (strLogConfigName.empty())
	{
		strLogConfigName = pPluginManager->GetAppName();
	}

	string strAppLogName = GetConfigPath(strLogConfigName);

	el::Configurations conf(strAppLogName);

	el::Configuration* pConfiguration = conf.get(el::Level::Debug, el::ConfigurationType::Filename);
	if (pConfiguration == nullptr)
	{
		conf = el::Configurations(GetConfigPath("Default"));
		pConfiguration = conf.get(el::Level::Debug, el::ConfigurationType::Filename);
	}

	const std::string& fileName = pConfiguration->value();
	pConfiguration->setValue(pPluginManager->GetConfigPath() + fileName);

#ifdef DEBUG
	std::cout << "LogConfig: " << strAppLogName << std::endl;
#endif
	el::Loggers::reconfigureAllLoggers(conf);
	el::Helpers::installPreRollOutCallback(rolloutHandler);


	return true;
}

bool LogModule::Start()
{
	m_pKernelModule = this->pPluginManager->FindModule<IKernelModule>();

    return true;
}

bool LogModule::Destory()
{
    el::Helpers::uninstallPreRollOutCallback();

    return true;
}

bool LogModule::BeforeDestory()
{
    return true;

}

bool LogModule::AfterStart()
{

    return true;
}

bool LogModule::Update()
{
    return true;

}

bool LogModule::Log(const SQUICK_LOG_LEVEL nll, const char* format, ...)
{
    mnLogCountTotal++;

    char szBuffer[1024 * 10] = {0};

    va_list args;
    va_start(args, format);
    vsnprintf(szBuffer, sizeof(szBuffer) - 1, format, args);
    va_end(args);

    mstrLocalStream.clear();

    mstrLocalStream.append(std::to_string(mnLogCountTotal));
    mstrLocalStream.append(" | ");
    mstrLocalStream.append(std::to_string(pPluginManager->GetAppID()));
    mstrLocalStream.append(" | ");
    mstrLocalStream.append(szBuffer);

    if (mLogHooker)
    {
        mLogHooker.get()->operator()(nll, mstrLocalStream);
    }

    switch (nll)
    {
        case ILogModule::NLL_DEBUG_NORMAL:
			{
				std::cout << termcolor::green;
				LOG(DEBUG) << mstrLocalStream;
			}
			break;
        case ILogModule::NLL_INFO_NORMAL:
			{
				std::cout << termcolor::green;
				LOG(INFO) << mstrLocalStream;
			}	
			break;
        case ILogModule::NLL_WARING_NORMAL:
			{
				std::cout << termcolor::yellow;
				LOG(WARNING) << mstrLocalStream;
			}
			break;
        case ILogModule::NLL_ERROR_NORMAL:
			{
				std::cout << termcolor::red;
				LOG(ERROR) << mstrLocalStream;
				//LogStack();
			}
			break;
        case ILogModule::NLL_FATAL_NORMAL:
			{
				std::cout << termcolor::red;
				LOG(FATAL) << mstrLocalStream;
			}
			break;
        default:
			{
				std::cout << termcolor::green;
				LOG(INFO) << mstrLocalStream;
			}
			break;
    }

	std::cout<<termcolor::reset;

    return true;
}

bool LogModule::LogRecord(const SQUICK_LOG_LEVEL nll, const Guid ident, const std::string& recordName, const std::string& strDesc, const char* func, int line)
{
	std::ostringstream os;
	auto record = m_pKernelModule->FindRecord(ident, recordName);
	if (record)
	{

		if (line > 0)
		{
			Log(nll, "[RECORD] Indent[%s] Record[%s] %s %s %d", ident.ToString().c_str(), recordName.c_str(), record->ToString().c_str(), func, line);
		}
		else
		{
			Log(nll, "[RECORD] Indent[%s] Record[%s] %s", ident.ToString().c_str(), recordName.c_str(), record->ToString().c_str());
		}
	}


    return true;
}

bool LogModule::LogObject(const SQUICK_LOG_LEVEL nll, const Guid ident, const std::string& strDesc, const char* func, int line)
{
    if (line > 0)
    {
        Log(nll, "[OBJECT] Indent[%s] %s %s %d", ident.ToString().c_str(), strDesc.c_str(), func, line);
    }
    else
    {
        Log(nll, "[OBJECT] Indent[%s] %s", ident.ToString().c_str(), strDesc.c_str());
    }

    return true;

}

void LogModule::LogStack()
{
#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
	Exception::CrashHandler(0);
#endif
}
bool LogModule::LogDebugFunctionDump(const Guid ident, const int nMsg, const std::string& strArg,  const char* func /*= ""*/, const int line /*= 0*/)
{
    //#ifdef SQUICK_DEBUG_MODE
    LogDebug(ident, strArg + "MsgID:" + std::to_string(nMsg), func, line);
    //#endif
    return true;
}

bool LogModule::ChangeLogLevel(const std::string& strLevel)
{
    el::Level logLevel = el::LevelHelper::convertFromString(strLevel.c_str());
    el::Logger* pLogger = el::Loggers::getLogger("default");
    if (NULL == pLogger)
    {
        return false;
    }

    el::Configurations* pConfigurations = pLogger->configurations();
    if (NULL == pConfigurations)
    {
        return false;
    }

    switch (logLevel)
    {
        case el::Level::Fatal:
        {
            el::Configuration errorConfiguration(el::Level::Error, el::ConfigurationType::Enabled, "false");
            pConfigurations->set(&errorConfiguration);
        }
        case el::Level::Error:
        {
            el::Configuration warnConfiguration(el::Level::Warning, el::ConfigurationType::Enabled, "false");
            pConfigurations->set(&warnConfiguration);
        }
        case el::Level::Warning:
        {
            el::Configuration infoConfiguration(el::Level::Info, el::ConfigurationType::Enabled, "false");
            pConfigurations->set(&infoConfiguration);
        }
        case el::Level::Info:
        {
            el::Configuration debugConfiguration(el::Level::Debug, el::ConfigurationType::Enabled, "false");
            pConfigurations->set(&debugConfiguration);

        }
        case el::Level::Debug:
            break;
        default:
            break;
    }

    el::Loggers::reconfigureAllLoggers(*pConfigurations);
    LogInfo("[Log] Change log level as " + strLevel, __FUNCTION__, __LINE__);
    return true;
}

bool LogModule::LogDebug(const std::string& strLog, const char* func , int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "%s %s %d", strLog.c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "%s", strLog.c_str());
     }

     return true;
}

bool LogModule::LogInfo(const std::string& strLog, const  char* func, int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "%s %s %d", strLog.c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "%s", strLog.c_str());
     }

     return true;
}

bool LogModule::LogWarning(const std::string& strLog, const char* func , int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "%s %s %d", strLog.c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "%s", strLog.c_str());
     }

     return true;
}

bool LogModule::LogError(const std::string& strLog, const char* func , int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "%s %s %d", strLog.c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "%s", strLog.c_str());
     }

     return true;
}

bool LogModule::LogFatal(const std::string& strLog, const char* func , int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "%s %s %d", strLog.c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "%s", strLog.c_str());
     }

     return true;
}


bool LogModule::LogDebug(const std::ostringstream& stream, const char* func , int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "%s %s %d", stream.str().c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "%s", stream.str().c_str());
     }

     return true;
}

bool LogModule::LogInfo(const std::ostringstream& stream, const  char* func, int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "%s %s %d", stream.str().c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "%s", stream.str().c_str());
     }

     return true;
}

bool LogModule::LogWarning(const std::ostringstream& stream, const char* func , int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "%s %s %d", stream.str().c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "%s", stream.str().c_str());
     }

     return true;
}

bool LogModule::LogError(const std::ostringstream& stream, const char* func , int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "%s %s %d", stream.str().c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "%s", stream.str().c_str());
     }

     return true;
}

bool LogModule::LogFatal(const std::ostringstream& stream, const char* func , int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "%s %s %d", stream.str().c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "%s", stream.str().c_str());
     }

     return true;
}


bool LogModule::LogDebug(const Guid ident, const std::string& strLog, const char* func , int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), strLog.c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), strLog.c_str());
     }

     return true;
}

bool LogModule::LogInfo(const Guid ident, const std::string& strLog, const  char* func, int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), strLog.c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), strLog.c_str());
     }

     return true;
}

bool LogModule::LogWarning(const Guid ident, const std::string& strLog, const char* func , int line)
{
    if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), strLog.c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), strLog.c_str());
     }

     return true;
}

bool LogModule::LogError(const Guid ident, const std::string& strLog, const char* func , int line)
{
     if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), strLog.c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), strLog.c_str());
     }

     return true;
}

bool LogModule::LogFatal(const Guid ident, const std::string& strLog, const char* func , int line)
{
     if (line > 0)
     {
         Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), strLog.c_str(), func, line);
     }
     else
     {
         Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), strLog.c_str());
     }

     return true;
}


bool LogModule::LogDebug(const Guid ident, const std::ostringstream& stream, const char* func , int line)
{
    if (line > 0)
    {
        Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), stream.str().c_str(), func, line);
    }
    else
    {
        Log(SQUICK_LOG_LEVEL::NLL_DEBUG_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), stream.str().c_str());
    }

    return true;
}

bool LogModule::LogInfo(const Guid ident, const std::ostringstream& stream, const  char* func, int line)
{
    if (line > 0)
    {
        Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), stream.str().c_str(), func, line);
    }
    else
    {
        Log(SQUICK_LOG_LEVEL::NLL_INFO_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), stream.str().c_str());
    }

    return true;
}

bool LogModule::LogWarning(const Guid ident, const std::ostringstream& stream, const char* func , int line)
{
    if (line > 0)
    {
        Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), stream.str().c_str(), func, line);
    }
    else
    {
        Log(SQUICK_LOG_LEVEL::NLL_WARING_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), stream.str().c_str());
    }

    return true;
}

bool LogModule::LogError(const Guid ident, const std::ostringstream& stream, const char* func , int line)
{
    if (line > 0)
    {
        Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), stream.str().c_str(), func, line);
    }
    else
    {
        Log(SQUICK_LOG_LEVEL::NLL_ERROR_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), stream.str().c_str());
    }

    return true;
}

bool LogModule::LogFatal(const Guid ident, const std::ostringstream& stream, const char* func , int line)
{
    if (line > 0)
    {
        Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "Indent[%s] %s %s %d", ident.ToString().c_str(), stream.str().c_str(), func, line);
    }
    else
    {
        Log(SQUICK_LOG_LEVEL::NLL_FATAL_NORMAL, "Indent[%s] %s", ident.ToString().c_str(), stream.str().c_str());
    }

    return true;
}

void LogModule::SetHooker(LOG_HOOKER_FUNCTOR_PTR hooker)
{
    mLogHooker = hooker;
}