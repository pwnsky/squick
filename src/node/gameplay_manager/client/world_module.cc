
#include "world_module.h"
#include "plugin.h"
#include <squick/struct/struct.h>
namespace gameplay_manager::client {
bool WorldModule::Start() { return true; }

bool WorldModule::Destory() { return true; }

bool WorldModule::AfterStart() { return true; }

bool WorldModule::BeforeDestory() { return false; }

bool WorldModule::Update() { return true; }

} // namespace gameplay_manager::client