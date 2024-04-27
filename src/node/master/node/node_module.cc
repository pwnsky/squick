
#include "node_module.h"
#include "plugin.h"

namespace master::node {
NodeModule::~NodeModule() {}

bool NodeModule::Destroy() { return true; }

bool NodeModule::AfterStart() {

    Listen();

    return true;
}

} // namespace master::node
