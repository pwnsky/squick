#pragma once
#include <squick/core/base.h>

namespace db_proxy::mongo {
    class Plugin : public IPlugin {
    public:
        Plugin(IPluginManager* p) { pPluginManager = p; }

        virtual const int GetPluginVersion();

        virtual const std::string GetPluginName();

        virtual void Install();

        virtual void Uninstall();
    };

}