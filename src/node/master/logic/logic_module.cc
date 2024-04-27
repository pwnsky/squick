
#include "logic_module.h"
#include "plugin.h"
#include <third_party/nlohmann/json.hpp>
#define DEFAULT_MASTER_UPDATE_SELF_TIME 10
namespace master::logic {
using json = nlohmann::json;
bool LogicModule::Start() {
    m_net_ = pm_->FindModule<INetModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_node_ = pm_->FindModule<INodeModule>();
    return true;
}

bool LogicModule::AfterStart() {
    m_net_->AddReceiveCallBack(rpc::IdNReqNodeRegister, this, &LogicModule::OnNReqNodeRegister);
    m_net_->AddReceiveCallBack(rpc::IdNReqNodeUnregister, this, &LogicModule::OnNReqNodeUnregistered);
    m_net_->AddReceiveCallBack(rpc::IdNNtfNodeReport, this, &LogicModule::OnNNtfNodeReport);
    m_net_->AddReceiveCallBack(rpc::IdNReqMinWorkloadNodeInfo, this, &LogicModule::OnNReqMinWorkNodeInfo);
    m_net_->AddReceiveCallBack(rpc::IdNNtfNodeMsgForward, this, &LogicModule::OnNNtfNodeMsgForward);
    m_net_->AddReceiveCallBack(rpc::IdNReqAllNodesInfo, this, &LogicModule::OnNReqAllNodesInfo);

    m_http_server_ = pm_->FindModule<::IHttpServerModule>();

    m_http_server_->AddRequestHandler("/node/list", HttpType::SQUICK_HTTP_REQ_GET, this, &LogicModule::OnGetNodeList);
    m_http_server_->StartServer(pm_->GetArg("http_port=", ARG_DEFAULT_HTTP_PORT));

    UpdateStatus();
    return true;
}

bool LogicModule::Destroy() { return true; }
bool LogicModule::Update() {

    if (last_report_time_ + DEFAULT_MASTER_UPDATE_SELF_TIME > pm_->GetNowTime()) {
        return true;
    }
    if (last_report_time_ > 0) {
        UpdateStatus();
    }
    last_report_time_ = pm_->GetNowTime();

    return true;
}

void LogicModule::UpdateStatus() { node_map_[pm_->GetAppID()] = m_node_->GetNodeInfo(); }

void LogicModule::OnNReqNodeRegister(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    uint64_t uid;
    rpc::NReqNodeRegister req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, uid)) {
        return;
    }
    rpc::NAckNodeRegister ack;
    int new_node_id = 0;
    ack.set_code(1);
    do {
        auto &new_node = req.node();
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
        LOG_INFO("Register node<%v>, subscribe node type:[ %v ]", new_node_id, log);
        AddSubscribeNode(new_node_id, types);

        // pack current subscribe nodes to new nodes
        for (auto s : node_map_) {
            auto s_info = s.second.info;
            // except self
            if (s_info->id() == new_node_id)
                continue;

            // except no subscribe type
            if (!IsHaveThisType(types, s_info->type()))
                continue;

            auto add_node = ack.add_node_add_list();
            *add_node = *s_info;
        }
        ack.set_code(0);
    } while (false);

    m_net_->SendPBToNode(rpc::IdNAckNodeRegister, ack, sock);

    NtfSubscribNode(new_node_id);
}

void LogicModule::AddSubscribeNode(int new_node_id, vector<int> types) {
    for (auto t : types) {
        if (t != 0) {
            nodes_subscribe_[t].insert(new_node_id);
        } else {
            LOG_ERROR("AddSubscribeNode error type<%v> ", t);
        }
    }
}

void LogicModule::NtfSubscribNode(int new_node_id) {
    if (new_node_id == 0)
        return;
    auto iter = node_map_.find(new_node_id);
    if (iter == node_map_.end()) {
        LOG_ERROR("NtfSubscribNode error id<%v>", new_node_id);
        return;
    }

    auto new_node = iter->second.info;
    int new_node_type = new_node->type();
    for (auto ns : nodes_subscribe_) {
        int type = ns.first;
        if (type != new_node_type)
            continue;
        for (auto sub_id : ns.second) {
            if (sub_id == new_node_id)
                continue;
            rpc::NNtfNodeAdd ntf;
            auto p = ntf.add_node_list();
            *p = *new_node;
            LOG_INFO("Ntf sub_id<%v> to add new_node<%v>", sub_id, new_node_id);
            SendPBByID(sub_id, rpc::IdNNtfNodeAdd, ntf);
        }
    }
}

bool LogicModule::SendPBByID(const int node_id, const uint16_t msg_id, const google::protobuf::Message &pb) {

    auto iter = node_map_.find(node_id);
    if (iter == node_map_.end()) {
        LOG_ERROR("SendPBByID no this node id<%v>", node_id);
        return false;
    }
    return m_net_->SendPBToNode(msg_id, pb, iter->second.fd);
}

void LogicModule::OnNReqNodeUnregistered(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
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
    // m_log_->LogInfo(Guid(0, id, s.name(), " UnRegistered");
}

// Master
void LogicModule::OnNNtfNodeReport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {

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

void LogicModule::OnNReqMinWorkNodeInfo(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
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
        if (id == -1)
            continue;
        auto p = ack.add_list();
        auto iter = node_map_.find(id);
        *p = *iter->second.info;
    }
    reqid_t req_id = msg_base.req_id();
    m_net_->SendPBToNode(rpc::IdNAckMinWorkloadNodeInfo, ack, sock, 0, req_id);
}

int LogicModule::GetLoadBanlanceNode(ServerType type) {
    int node_id = -1;
    int min_workload = 0x7fffffff;
    for (auto &iter : node_map_) {
        auto server = iter.second;
        if (server.info->type() == type) {
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

void LogicModule::OnNNtfNodeMsgForward(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void LogicModule::OnNReqAllNodesInfo(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    rpc::MsgBase msg_base;
    if (!msg_base.ParseFromArray(msg, len)) {
        LOG_ERROR("ParseFromArray: is error, the msg len %v", len);
        return;
    }
    reqid_t req_id = msg_base.req_id();
    rpc::NAckAllNodesInfo ack;
    for (auto &node : node_map_) {
        auto node_info = ack.add_node_list();
        *node_info = *node.second.info;
    }
    m_net_->SendPBToNode(rpc::IdNAckMinWorkloadNodeInfo, ack, sock, 0, req_id);
}

bool LogicModule::OnGetNodeList(std::shared_ptr<HttpRequest> req) {

    json statusRoot;
    statusRoot["code"] = 0;
    statusRoot["msg"] = "";
    statusRoot["time"] = pm_->GetNowTime();
    for (auto &s : node_map_) {
        auto &sd = s.second.info;
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
        n["ws_port"] = sd->ws_port();
        n["http_port"] = sd->http_port();
        n["https_port"] = sd->https_port();
        n["connections"] = sd->connections();
        n["net_client_connections"] = sd->net_client_connections();

        statusRoot["node_list"][to_string(sd->id())] = n;
    }
    return m_http_server_->ResponseMsg(req, statusRoot.dump(), WebStatus::WEB_OK);
}

} // namespace master::logic
