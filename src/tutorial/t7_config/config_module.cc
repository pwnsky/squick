#include "config_module.h"

namespace tutorial {
bool SimpleModule::Start() { return true; }

bool SimpleModule::AfterStart() { return true; }

bool SimpleModule::Update() { return true; }

} // namespace tutorial