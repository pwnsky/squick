#include "node_module.h"
#include <forward_list>

namespace room::node {
bool NodeModule::AfterStart() {
    Listen();

    vector<int> node_types = { ServerType::ST_GLOBAL, ServerType::ST_DB_PROXY };
    AddNodesByType(node_types);
    return true;
}

bool NodeModule::Destroy() { return true; }

}