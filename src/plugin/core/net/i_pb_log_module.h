#pragma once
#include <core/i_module.h>

class IPbLogModule : public IModule {
  public:
    virtual void Log(const std::string &prefix, const uint32_t msg_id, const char *data, const uint32_t length) = 0;
};