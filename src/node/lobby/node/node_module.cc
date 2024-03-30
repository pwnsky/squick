#include "node_module.h"
#include <forward_list>

namespace lobby::node {
bool NodeModule::AfterStart() {
    Listen();
    //AddServer(ServerType::ST_WORLD);
    //AddServer(ServerType::ST_DB_PROXY);

    ConnectToMaster();
    return true;
}

bool NodeModule::Destory() { return true; }

} // namespace lobby::server