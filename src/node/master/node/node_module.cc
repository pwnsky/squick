
#include "node_module.h"
#include "plugin.h"
#include <third_party/nlohmann/json.hpp>

namespace master::node {
    NodeModule::~NodeModule() {}

bool NodeModule::Destory() { return true; }

bool NodeModule::AfterStart() {
    
    m_net_->AddReceiveCallBack(rpc::MasterRPC::NN_REQ_NODE_REGISTER, this, &NodeModule::OnNnReqNodeRegister);
    m_net_->AddReceiveCallBack(rpc::MasterRPC::NN_REQ_NODE_UNREGISTER, this, &NodeModule::OnNnReqNodeUnregistered);
    m_net_->AddReceiveCallBack(rpc::MasterRPC::NN_NTF_NODE_REPORT, this, &NodeModule::OnReport);
    Listen();
    
    return true;
}

void NodeModule::OnNnReqNodeRegister(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    string nPlayerID;
    rpc::NnReqNodeRegister req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, nPlayerID)) {
        return;
    }
    rpc::NnAckNodeRegister ack;
    ack.set_code(1);
    do {
        auto& cs = servers_[pm_->GetAppID()];
        
        auto& new_node = req.node();

        if (new_node.id() == pm_->GetAppID()) {
            break;
        }
        ServerInfo info;
        info.fd = sock;
        info.status = ServerInfo::Status::Connected;
        *info.info = new_node;
        servers_[new_node.id()] = info;

        ack.set_code(0);
    } while (false);
    m_net_->SendMsgPB(rpc::MasterRPC::NN_ACK_NODE_REGISTER, ack, sock);
}

void NodeModule::OnNnReqNodeUnregistered(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    string guid;
    rpc::NnReqNodeUnregister req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, guid)) {
        return;
    }
    
    int id = req.id();
    auto iter = servers_.find(id);
    if (iter != servers_.end()) {
        servers_.erase(iter);
    }
    //m_log_->LogInfo(Guid(0, id, s.name(), " UnRegistered");
}

// Master
void NodeModule::OnReport(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    
    string guid;
    rpc::NnNtfNodeReport ntf;
    if (!INetModule::ReceivePB(msg_id, msg, len, ntf, guid)) {
        return;
    }

    do {
        for (auto s : ntf.list()) {
            if (s.id() == pm_->GetAppID() || s.id() == 0) {
                continue;
            }
            auto iter = servers_.find(s.id());
            if (iter != servers_.end()) {
                if (iter->second.info->update_time() >= s.update_time()) {
                    continue;
                }
                *iter->second.info = s;
                continue;
            }

            // New
            ServerInfo info;
            info.fd = 0;
            info.status = ServerInfo::Status::Unknowing;
            *info.info = s;
            servers_[s.id()] = info;
        }
    } while (false);
}


int NodeModule::GetLoadBanlanceNode(ServerType type) {
    int node_id = -1;
    int min_workload = 99999;
    for (auto& iter : servers_) {
        auto server = iter.second;
        if (server.info->type() == type && server.info->area() == pm_->GetArea()) {
            if (min_workload > server.info->workload()) {
                node_id = iter.first;
            }
        }
    }
    if (node_id == -1) {
        return -1;
    }
    return node_id;
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
        json n;
        n["area"] = sd->area();
        n["type"] = sd->type();
        n["id"] = sd->id();
        n["name"] = sd->name().c_str();
        n["ip"] = sd->ip().c_str();
        n["public_ip"] = sd->public_ip().c_str();
        n["port"] = sd->port();
        n["cpu_count"] = sd->cpu_count();
        n["status"] = sd->state();
        n["workload"] = sd->workload();
        n["update_time"] = sd->update_time();
        statusRoot["node_list"][to_string(sd->id())] = n;
    }
    return statusRoot.dump();
}

map<int, ServerInfo>& NodeModule::GetServers() { return servers_; }

} // namespace master::server
