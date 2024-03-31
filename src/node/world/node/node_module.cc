
#include "node_module.h"
#include "plugin.h"
#include <struct/struct.h>
namespace world::node {
bool NodeModule::AfterStart() {
	Listen();

	vector<int> node_types = {};
	AddNodesByType(node_types);
	return true;
}
bool NodeModule::Destory() { return true; }

} // namespace world::server