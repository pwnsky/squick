#pragma once

#include <core/base.h>
namespace squick::thread {
class Plugin : public IPlugin {
  public:
    Plugin(IPluginManager *p) { pm_ = p; }
    virtual const int GetPluginVersion();
    virtual const std::string GetPluginName();
    virtual void Install();
    virtual void Uninstall();
};
} // namespace squick::thread