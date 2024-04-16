
#pragma once

#include <squick/core/i_module.h>
#include <squick/core/easylogging++.h>
#include <squick/core/base.h>

#ifdef SQUICK_DEV
    #define LOG_DEBUG(format, ...)  m_log_->GetLogger()->debug ("[%v][%v:%v] [%v] " format, pm_->GetAppName(), __FUNCTION__, __LINE__, this->name_, __VA_ARGS__)
    #define LOG_INFO(format, ...)   m_log_->GetLogger()->info  ("[%v][%v:%v] [%v] " format, pm_->GetAppName(), __FUNCTION__, __LINE__, this->name_, __VA_ARGS__)

    #define LOG_WARN(format, ...)   \
	std::cout << termcolor::yellow; \
	m_log_->GetLogger()->warn  ("[%v][%v:%v] [%v] " format, pm_->GetAppName(), __FUNCTION__, __LINE__, this->name_, __VA_ARGS__); \
	std::cout << termcolor::reset

    #define LOG_ERROR(format, ...)  \
	std::cout << termcolor::red;    \
	m_log_->GetLogger()->error ("[%v][%v:%v] [%v] " format, pm_->GetAppName(), __FUNCTION__, __LINE__, this->name_, __VA_ARGS__); \
	std::cout << termcolor::reset

    #define LOG_FATAL(format, ...)  \
	std::cout << termcolor::red;    \
	m_log_->GetLogger()->fatal ("[%v][%v:%v] [%v] " format, pm_->GetAppName(), __FUNCTION__, __LINE__, this->name_, __VA_ARGS__); \
	std::cout << termcolor::reset

#else
    #define LOG_DEBUG(format, ...)  m_log_->GetLogger()->debug ("[%v] " format, this->name_, __VA_ARGS__)
    #define LOG_INFO(format, ...)   m_log_->GetLogger()->info  ("[%v] " format, this->name_, __VA_ARGS__)
    #define LOG_ERROR(format, ...)  m_log_->GetLogger()->error ("[%v] " format, this->name_, __VA_ARGS__)
    #define LOG_WARN(format, ...)   m_log_->GetLogger()->warn  ("[%v] " format, this->name_, __VA_ARGS__)
    #define LOG_FATAL(format, ...)  m_log_->GetLogger()->fatal ("[%v] " format, this->name_, __VA_ARGS__)
#endif

class ILogModule : public IModule {
  public:

    virtual el::Logger* GetLogger() = 0;
};
