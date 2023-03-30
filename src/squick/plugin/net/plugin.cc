
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
    REGISTER_MODULE(pm_, INetModule, NetModule)
    REGISTER_MODULE(pm_, IWSModule, WSModule)
    REGISTER_MODULE(pm_, IHttpServerModule, HttpServerModule)
    REGISTER_MODULE(pm_, INetClientModule, NetClientModule)
    REGISTER_MODULE(pm_, IHttpClientModule, HttpClientModule)
    // REGISTER_MODULE(pm_, IUDPModule, UDPModule)
}

void NetPlugin::Uninstall() {
    // UNREGISTER_MODULE(pm_, IUDPModule, UDPModule)
    UNREGISTER_MODULE(pm_, IHttpClientModule, HttpClientModule)
    UNREGISTER_MODULE(pm_, INetClientModule, NetClientModule)
    UNREGISTER_MODULE(pm_, IHttpServerModule, HttpServerModule)
    UNREGISTER_MODULE(pm_, IWSModule, WSModule)
    UNREGISTER_MODULE(pm_, INetModule, NetModule)
}