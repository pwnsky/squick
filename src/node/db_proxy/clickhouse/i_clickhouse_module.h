#pragma once
#include <squick/core/base.h>

class IClickhouseModule : public IModule {
  public:
    virtual bool Connect(std::string url) = 0;
};
