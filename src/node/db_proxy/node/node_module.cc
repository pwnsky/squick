#include <struct/struct.h>

//#include "../logic/player_redis_module.h"
#include "node_module.h"

namespace db_proxy::node {

bool NodeModule::AfterStart() {
    
    Listen();
    vector<int> node_types = { ServerType::ST_WORLD, ServerType::ST_LOBBY };
    AddNodesByType(node_types);
    return true;
}

bool NodeModule::Destory() { return true; }


void NodeModule::OnClientDisconnect(const socket_t sock) {}

void NodeModule::OnClientConnected(const socket_t sock) {}

} // namespace db_proxy::server