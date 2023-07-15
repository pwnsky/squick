#include "manager_module.h"
#include <struct/struct.h>
// #include <third_party/nlohmann/json.hpp>
namespace gameplay_manager::logic {
bool ManagerModule::Start() {
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    return true;
}

bool ManagerModule::Destory() { return true; }

bool ManagerModule::AfterStart() { return true; }

bool ManagerModule::Update() { return true; }

} // namespace gameplay_manager::logic