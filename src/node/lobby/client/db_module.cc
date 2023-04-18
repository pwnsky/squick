

#include "db_module.h"
#include "plugin.h"
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/struct/struct.h>

namespace lobby::client {
bool DBModule::Start() { return true; }

bool DBModule::Destory() { return true; }

bool DBModule::Update() { return true; }

bool DBModule::AfterStart() { return true; }

} // namespace lobby::client