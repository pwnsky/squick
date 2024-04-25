#include "node_module.h"
#include <forward_list>

namespace web::node {
bool NodeModule::AfterStart() {
    Listen();

    vector<int> node_types = {ServerType::ST_GLOBAL, ServerType::ST_DB_PROXY, ServerType::ST_PLAYER};
    AddNodesByType(node_types);
    return true;
}

bool NodeModule::Destroy() { return true; }

} // namespace web::node
