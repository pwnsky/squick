
#include "node_module.h"
#include "plugin.h"
#include <third_party/nlohmann/json.hpp>

namespace master::node {
    NodeModule::~NodeModule() {}

bool NodeModule::Destory() { return true; }

bool NodeModule::AfterStart() {
    
    m_net_->AddReceiveCallBack(rpc::MasterRPC::NN_REQ_NODE_REGISTER, this, &NodeModule::OnNnReqNodeRegister);
    m_net_->AddReceiveCallBack(rpc::MasterRPC::NN_REQ_NODE_UNREGISTER, this, &NodeModule::OnNnReqNodeUnregistered);
    m_net_->AddReceiveCallBack(rpc::MasterRPC::NN_NTF_NODE_REPORT, this, &NodeModule::OnNnNtfNodeReport);
    m_net_->AddReceiveCallBack(rpc::MasterRPC::NN_REQ_MIN_WORKLOAD_NODE_INFO, this, &NodeModule::OnNnReqMinWorkNodeInfo);
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
    int new_node_id = 0;
    ack.set_code(1);
    do {        
        auto& new_node = req.node();
        new_node_id = new_node.id();
        if (new_node_id == pm_->GetAppID() || new_node_id == 0) {
            break;
        }
        ServerInfo info;
        info.fd = sock;
        info.status = ServerInfo::Status::Connected;
        *info.info = new_node;
        node_map_[new_node_id] = info;

        vector<int> types;
        std::string log;
        for (auto t : req.listen_type_list()) {
            types.push_back(t);
            log += ", " + to_string(t);
        }
        dout << "Register node: " << new_node_id << " subscribe node type: [" << log << "]" << endl;
        AddSubscribeNode(new_node_id, types);

        // pack current subscribe nodes to new nodes
        for (auto s : node_map_) {
            auto s_info = s.second.info;
            // except self
            if (s_info->id() == new_node_id) continue;

            // except no subscribe type
            if(!IsHaveThisType(types, s_info->type())) continue;

            auto add_node = ack.add_node_add_list();
            *add_node = *s_info;
        }
        ack.set_code(0);
    } while (false);

    m_net_->SendMsgPB(rpc::MasterRPC::NN_ACK_NODE_REGISTER, ack, sock);

    NtfSubscribNode(new_node_id);
}

void NodeModule::AddSubscribeNode(int new_node_id, vector<int> types) {
    for (auto t : types) {
        if (t != 0) {
            nodes_subscribe_[t].insert(new_node_id);
        } else {
            m_log_->LogError("AddSubscribeNode error type: " + to_string(t));
        }
    }
}

void NodeModule::NtfSubscribNode(int new_node_id) {
    if (new_node_id == 0) return;
    auto iter = node_map_.find(new_node_id);
    if (iter == node_map_.end()) {
        m_log_->LogError("NtfSubscribNode error id: " + to_string(new_node_id));
        return;
    }

    auto new_node = iter->second.info;
    int new_node_type = new_node->type();
    for (auto ns : nodes_subscribe_) {
        int type = ns.first;
        if (type != new_node_type) continue;
        for (auto sub_id : ns.second) {
            if (sub_id == new_node_id) continue;
            rpc::NnNtfNodeAdd ntf;
            auto p = ntf.add_node_list();
            *p = *new_node;
            dout << "Ntf add: " << sub_id << " new: " << new_node_id << endl;
            SendPBByID(sub_id, rpc::MasterRPC::NN_NTF_NODE_ADD, ntf);
        }
    }
}

bool NodeModule::SendPBByID(const int node_id, const uint16_t msg_id, const google::protobuf::Message& pb) {
    
    auto iter = node_map_.find(node_id);
    if (iter == node_map_.end()) {
        m_log_->LogError("SendPBByID no this node id: " + to_string(node_id));
        return false;
    }
    return m_net_->SendMsgPB(msg_id, pb, iter->second.fd);
}

void NodeModule::OnNnReqNodeUnregistered(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    string guid;
    rpc::NnReqNodeUnregister req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, guid)) {
        return;
    }
    
    int id = req.id();
    auto iter = node_map_.find(id);
    if (iter != node_map_.end()) {
        node_map_.erase(iter);
    }
    //m_log_->LogInfo(Guid(0, id, s.name(), " UnRegistered");
}

// Master
void NodeModule::OnNnNtfNodeReport(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    
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
            auto iter = node_map_.find(s.id());
            if (iter != node_map_.end()) {
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
            node_map_[s.id()] = info;
        }
    } while (false);
}

void NodeModule::OnNnReqMinWorkNodeInfo(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    rpc::NnReqMinWorkloadNodeInfo req;
    rpc::NnAckMinWorkloadNodeInfo ack;
    string guid;
    
    rpc::MsgBase msg_base;
    if (!msg_base.ParseFromArray(msg, len)) {
        return;
    }
    if (!req.ParseFromString(msg_base.msg_data())) {
        return;
    }
    dout << "find ....\n";
    for (auto type : req.type_list()) {
        int id = GetLoadBanlanceNode((ServerType)type);
        if (id == -1) continue;
        dout << " added min id: " << id << endl;
        auto p = ack.add_list();
        auto iter = node_map_.find(id);
        *p = *iter->second.info;
    }
    reqid_t req_id = msg_base.req_id();
    m_net_->SendMsgPB(rpc::MasterRPC::NN_ACK_MIN_WORKLOAD_NODE_INFO, ack, sock, "", req_id);
}

int NodeModule::GetLoadBanlanceNode(ServerType type) {
    int node_id = -1;
    int min_workload = 99999;
    for (auto& iter : node_map_) {
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
    for (auto &s : node_map_) {
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
        n["max_online"] = sd->max_online();
        n["update_time"] = sd->update_time();
        statusRoot["node_list"][to_string(sd->id())] = n;
    }
    return statusRoot.dump();
}

map<int, ServerInfo>& NodeModule::GetServers() { return node_map_; }

} // namespace master::server
