
#include "node_module.h"

namespace game::node {

bool NodeModule::Destory() { return true; }

bool NodeModule::AfterStart() {

	Listen();
	AddServer(ST_WORLD);

	return true;
}

void NodeModule::OnClientDisconnect(socket_t sock) {

}

void NodeModule::OnClientConnected(socket_t sock) {

}

} // namespace game::node
