
#include "plugin.h"
#include "http_client_module.h"
#include "http_server_module.h"
#include "net_client_module.h"
#include "net_module.h"
#include "udp_module.h"
#include "ws_module.h"
#include <squick/core/i_plugin_manager.h>

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, NetPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, NetPlugin)};

//////////////////////////////////////////////////////////////////////////

const int NetPlugin::GetPluginVersion() { return 0; }

const std::string NetPlugin::GetPluginName() { return GET_CLASS_NAME(NetPlugin); }

void NetPlugin::Install() {
    REGISTER_MODULE(pPluginManager, INetModule, NetModule)
    REGISTER_MODULE(pPluginManager, IWSModule, WSModule)
    REGISTER_MODULE(pPluginManager, IHttpServerModule, HttpServerModule)
    REGISTER_MODULE(pPluginManager, INetClientModule, NetClientModule)
    REGISTER_MODULE(pPluginManager, IHttpClientModule, HttpClientModule)
    // REGISTER_MODULE(pPluginManager, IUDPModule, UDPModule)
}

void NetPlugin::Uninstall() {
    // UNREGISTER_MODULE(pPluginManager, IUDPModule, UDPModule)
    UNREGISTER_MODULE(pPluginManager, IHttpClientModule, HttpClientModule)
    UNREGISTER_MODULE(pPluginManager, INetClientModule, NetClientModule)
    UNREGISTER_MODULE(pPluginManager, IHttpServerModule, HttpServerModule)
    UNREGISTER_MODULE(pPluginManager, IWSModule, WSModule)
    UNREGISTER_MODULE(pPluginManager, INetModule, NetModule)
}