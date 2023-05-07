
#include "node_module.h"
#include "plugin.h"
#include <third_party/nlohmann/json.hpp>

namespace master::node {
    NodeModule::~NodeModule() {}

bool NodeModule::Destory() { return true; }

bool NodeModule::AfterStart() {
    m_net_->AddReceiveCallBack(rpc::ServerRPC::SERVER_HEARTBEAT, this, &NodeModule::OnHeartBeat);
    m_net_->AddReceiveCallBack(this, &NodeModule::InvalidMessage);
    Listen();
    return true;
}


void NodeModule::OnHeartBeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void NodeModule::InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { printf("Net || umsg_id=%d\n", msg_id); }


// 获取服务状态
std::string NodeModule::GetServersStatus() {
   
    using json = nlohmann::json;
    json statusRoot;
    
    statusRoot["code"] = 0;
    statusRoot["msg"] = "";
    statusRoot["time"] = pm_->GetNowTime();
    /*
    for (auto &iter : servers_) {
        auto &sd = iter.second.pData;
        json s;
        s["id"] = sd->id();
        s["name"] = sd->name().c_str();
        s["ip"] = sd->ip().c_str();
        s["port"] = sd->port();
        s["cpu_count"] = sd->cpu_count();
        s["status"] = sd->state();
        statusRoot[sd->id()] = s;
    }
    */
    return statusRoot.dump();
}

} // namespace master::server