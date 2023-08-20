
#include "node_module.h"
#include "plugin.h"
#include <third_party/nlohmann/json.hpp>

namespace master::node {
    NodeModule::~NodeModule() {}

bool NodeModule::Destory() { return true; }

bool NodeModule::AfterStart() {
    
    Listen();
    return true;
}

// 获取服务状态
std::string NodeModule::GetServersStatus() {
   
    using json = nlohmann::json;
    json statusRoot;
    
    statusRoot["code"] = 0;
    statusRoot["msg"] = "";
    statusRoot["time"] = pm_->GetNowTime();
    for (auto &s : servers_) {
        auto& sd = s.second.info;
        json s;
        s["area"] = sd->area();
        s["type"] = sd->type();
        s["id"] = sd->id();
        s["name"] = sd->name().c_str();
        s["ip"] = sd->ip().c_str();
        s["port"] = sd->port();
        s["cpu_count"] = sd->cpu_count();
        s["status"] = sd->state();
        s["update_time"] = sd->update_time();
        statusRoot["node_list"][to_string(sd->id())] = s;
    }
    return statusRoot.dump();
}

map<int, ServerInfo>& NodeModule::GetServers() { return servers_; }

} // namespace master::server