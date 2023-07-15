
#include "node_module.h"
#include "plugin.h"
#include <struct/struct.h>
namespace world::node {
bool NodeModule::AfterStart() {
	Listen();

	AddServer(ServerType::ST_MASTER);

	return true;
}
bool NodeModule::Destory() { return true; }

} // namespace world::server