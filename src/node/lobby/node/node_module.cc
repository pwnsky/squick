#include "node_module.h"
#include <forward_list>
#include <squick/plugin/kernel/i_event_module.h>
#include <squick/plugin/kernel/scene_module.h>

namespace lobby::node {
bool NodeModule::AfterStart() {
    Listen();
    AddServer(ServerType::ST_WORLD);
    AddServer(ServerType::ST_DB_PROXY);
    return true;
}

bool NodeModule::Destory() { return true; }

} // namespace lobby::server