#pragma once
#include <core/base.h>

namespace db_proxy::mongo {
class Plugin : public IPlugin {
  public:
    Plugin(IPluginManager *p) { pm_ = p; }

    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};

} // namespace db_proxy::mongo