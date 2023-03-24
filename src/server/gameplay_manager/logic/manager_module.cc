#include "manager_module.h"
#include <squick/struct/struct.h>
// #include <third_party/nlohmann/json.hpp>
namespace gameplay_manager::logic {
bool ManagerModule::Start() {
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    return true;
}

bool ManagerModule::Destory() { return true; }

bool ManagerModule::AfterStart() {
    return true;
}

bool ManagerModule::Update() { return true; }

}