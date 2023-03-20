

#include "lua_bind_module.h"
#include <squick/plugin/lua/export.h>
// #include <third_party/nlohmann/json.hpp>
namespace gateway::lua {
bool LuaBindModule::Start() {
    m_pHttpNetModule = pPluginManager->FindModule<IHttpServerModule>();
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pLogicClassModule = pPluginManager->FindModule<IClassModule>();
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pLuaScriptModule = pPluginManager->FindModule<ILuaScriptModule>();
    return true;
}

bool LuaBindModule::Destory() { return true; }

bool LuaBindModule::AfterStart() { return true; }

bool LuaBindModule::Update() { return true; }

bool LuaBindModule::Bind() { return true; }

} // namespace gateway::lua