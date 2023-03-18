#include "manager_module.h"
#include <squick/struct/struct.h>
// #include <squick/plugin/lua/export.h>
// #include <third_party/nlohmann/json.hpp>
namespace gameplay_manager::logic {
bool ManagerModule::Start() {
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pLuaScriptModule = pPluginManager->FindModule<ILuaScriptModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    return true;
}

bool ManagerModule::Destory() { return true; }

bool ManagerModule::AfterStart() {
    std::cout << "启动Gameplay 管理模块\n";
    return true;
}

bool ManagerModule::Update() { return true; }

} // namespace gameplay_manager::logic