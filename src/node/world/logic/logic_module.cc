#include "logic_module.h"

namespace world::logic {
bool LogicModule::Start() {
    m_node_ = pm_->FindModule<INodeModule>();
    vector<int> node_types = { ServerType::ST_DB_PROXY };
    m_node_->AddSubscribeNode(node_types);
    return true;
}
}