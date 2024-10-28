
#include "plugin.h"
#include "lua_pb_module.h"
#include "lua_script_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){CREATE_PLUGIN(pm, LuaScriptPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, LuaScriptPlugin)};

//////////////////////////////////////////////////////////////////////////

const int LuaScriptPlugin::GetPluginVersion() { return 0; }

const std::string LuaScriptPlugin::GetPluginName() { return GET_CLASS_NAME(LuaScriptPlugin); }

void LuaScriptPlugin::Install() {
    REGISTER_MODULE(pm_, ILuaScriptModule, LuaScriptModule)
    REGISTER_MODULE(pm_, ILuaPBModule, LuaPBModule)
}

void LuaScriptPlugin::Uninstall() {
    UNREGISTER_MODULE(pm_, ILuaPBModule, LuaPBModule)
    UNREGISTER_MODULE(pm_, ILuaScriptModule, LuaScriptModule)
}