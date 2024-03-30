
#include "node_module.h"
#include "plugin.h"

namespace login::node {

bool NodeModule::Destory() { return true; }

bool NodeModule::BeforeDestory() { return true; }

bool NodeModule::AfterStart() {
    m_net_->AddReceiveCallBack(this, &NodeModule::InvalidMessage);
    Listen();
    ConnectToMaster();
    return true;
}

void NodeModule::OnClientConnected(const socket_t sock) {}

void NodeModule::InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { printf("Net || umsg_id=%d\n", msg_id); }

map<int, ServerInfo> &NodeModule::GetServers() { return servers_; }

} // namespace login::node