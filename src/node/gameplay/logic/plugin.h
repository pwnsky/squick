#pragma once

#include <squick/core/base.h>

namespace gameplay::play {
class Plugin : public IPlugin {
  public:
    Plugin(IPluginManager *p) { pm_ = p; }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};

} // namespace gameplay::play