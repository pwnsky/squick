#include "node_module.h"
#include <forward_list>

namespace lobby::node {
bool NodeModule::AfterStart() {
    Listen();

    vector<int> node_types = { ServerType::ST_WORLD, ServerType::ST_DB_PROXY, ServerType::ST_LOBBY };
    AddNodesByType(node_types);
    return true;
}

bool NodeModule::Destory() { return true; }

} // namespace lobby::server