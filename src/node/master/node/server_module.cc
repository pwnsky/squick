
#include "server_module.h"
#include "plugin.h"
#include <third_party/nlohmann/json.hpp>

namespace master::server {
ServerModule::~ServerModule() {}

bool ServerModule::Start() { return true; }

bool ServerModule::Destory() { return true; }

bool ServerModule::Update() {
    static time_t last = 0;
    time_t now = SquickGetTimeS();
    if (now - last > 10) {
        last = now;
    }
    return true;
}

bool ServerModule::AfterStart() {
    m_net_->AddReceiveCallBack(rpc::ServerRPC::SERVER_HEARTBEAT, this, &ServerModule::OnHeartBeat);
    m_net_->AddReceiveCallBack(this, &ServerModule::InvalidMessage);
    Listen();
    return true;
}

void ServerModule::OnClientDisconnect(const socket_t sock) {}

void ServerModule::OnClientConnected(const socket_t sock) {}

void ServerModule::OnHeartBeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void ServerModule::InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { printf("Net || umsg_id=%d\n", msg_id); }

void ServerModule::OnServerReport(const socket_t nFd, const int msg_id, const char *buffer, const uint32_t len) {}

// 获取服务状态
std::string ServerModule::GetServersStatus() {
    using json = nlohmann::json;
    json statusRoot;

    statusRoot["code"] = 0;
    statusRoot["msg"] = "";
    statusRoot["time"] = pm_->GetNowTime();

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

    return statusRoot.dump();
}

} // namespace master::server