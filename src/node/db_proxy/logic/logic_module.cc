#include "logic_module.h"

namespace db_proxy::logic {
bool LogicModule::Start() {
    m_node_ = pm_->FindModule<INodeModule>();
    vector<int> node_types = { };
    m_node_->AddSubscribeNode(node_types);
    return true;
}
}