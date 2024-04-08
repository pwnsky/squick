#include "node_module.h"
#include <forward_list>

namespace player::node {
bool NodeModule::AfterStart() {
    Listen();

    vector<int> node_types = { ServerType::ST_WORLD, ServerType::ST_DB_PROXY, ServerType::ST_PLAYER };
    AddNodesByType(node_types);
    return true;
}

bool NodeModule::Destory() { return true; }

} // namespace player::server