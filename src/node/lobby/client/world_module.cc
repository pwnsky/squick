

#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include "plugin.h"
#include "world_module.h"
namespace lobby::client {
bool WorldModule::Start() { return true; }

bool WorldModule::Destory() { return true; }

bool WorldModule::Update() { return true; }

bool WorldModule::AfterStart() { return true; }

} // namespace lobby::client
