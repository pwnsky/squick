
#include "node_module.h"
#include "plugin.h"

namespace master::node {
    NodeModule::~NodeModule() {}

bool NodeModule::Destroy() { return true; }

bool NodeModule::AfterStart() {
    
    m_net_->AddReceiveCallBack(rpc::NMasterRPC::NREQ_NODE_REGISTER, this, &NodeModule::OnNReqNodeRegister);
    m_net_->AddReceiveCallBack(rpc::NMasterRPC::NREQ_NODE_UNREGISTER, this, &NodeModule::OnNReqNodeUnregistered);
    m_net_->AddReceiveCallBack(rpc::NMasterRPC::NNTF_NODE_REPORT, this, &NodeModule::OnNNtfNodeReport);
    m_net_->AddReceiveCallBack(rpc::NMasterRPC::NREQ_MIN_WORKLOAD_NODE_INFO, this, &NodeModule::OnNReqMinWorkNodeInfo);
    Listen();
    
    return true;
}

void NodeModule::OnNReqNodeRegister(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    uint64_t uid;
    rpc::NReqNodeRegister req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, uid)) {
        return;
    }
    rpc::NAckNodeRegister ack;
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
        m_log_->LogInfo("Register node: " + std::to_string(new_node_id) + " subscribe node type: [" + log + "]");
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

    m_net_->SendMsgPB(rpc::NMasterRPC::NACK_NODE_REGISTER, ack, sock);

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
            rpc::NNtfNodeAdd ntf;
            auto p = ntf.add_node_list();
            *p = *new_node;
            m_log_->LogInfo("Ntf add: " + std::to_string(sub_id) + " new: " + std::to_string(new_node_id));
            SendPBByID(sub_id, rpc::NMasterRPC::NNTF_NODE_ADD, ntf);
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

void NodeModule::OnNReqNodeUnregistered(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    uint64_t uid;
    rpc::NReqNodeUnregister req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, uid)) {
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
void NodeModule::OnNNtfNodeReport(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    
    uint64_t uid;
    rpc::NNtfNodeReport ntf;
    if (!INetModule::ReceivePB(msg_id, msg, len, ntf, uid)) {
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

void NodeModule::OnNReqMinWorkNodeInfo(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    rpc::NReqMinWorkloadNodeInfo req;
    rpc::NAckMinWorkloadNodeInfo ack;
    string guid;
    
    rpc::MsgBase msg_base;
    if (!msg_base.ParseFromArray(msg, len)) {
        return;
    }
    if (!req.ParseFromString(msg_base.msg_data())) {
        return;
    }
    for (auto type : req.type_list()) {
        int id = GetLoadBanlanceNode((ServerType)type);
        if (id == -1) continue;
        auto p = ack.add_list();
        auto iter = node_map_.find(id);
        *p = *iter->second.info;
    }
    reqid_t req_id = msg_base.req_id();
    m_net_->SendMsgPB(rpc::NMasterRPC::NACK_MIN_WORKLOAD_NODE_INFO, ack, sock, 0, req_id);
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

map<int, ServerInfo>& NodeModule::GetAllNodes() { return node_map_; }

} // namespace master::server
