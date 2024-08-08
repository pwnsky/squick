
#include "node_module.h"

NodeModule::NodeModule(IPluginManager *p) {
    pm_ = p;
    is_update_ = true;
}

bool NodeModule::Awake() {
    m_net_ = pm_->FindModule<INetModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    is_update_ = true;
    pm_->SetAppType(StringNodeTypeToEnum(pm_->GetArg("type=", "proxy")));
    pm_->SetArea(pm_->GetArg("area=", 0));
    return true;
}

bool NodeModule::Start() {
    Listen();
    return true;
}

bool NodeModule::Update() {

    if (last_report_time_ + NODE_REPORT_TIME > pm_->GetNowTime()) {
        return true;
    }
    if (last_report_time_ > 0) {
        // calc work load
        CalcWorkLoad();
        UpdateState();
    }
    last_report_time_ = pm_->GetNowTime();
    return true;
}

inline void NodeModule::CalcWorkLoad() {
    // handle time * 100 + m_net connections + m_net_client connections
    time_t now_time = SquickGetTimeMS();
    // The first update will int overflow, but who care?
    workload_ = (now_time - last_update_time_) * 100;
    int total_connections = m_net_->GetNet()->GetConnections() + m_net_client_->GetConnections();
    workload_ += ONE_CONNECTION_WORKLOAD_VALUE * total_connections;
    last_update_time_ = now_time;
}

int NodeModule::CalcConnectionWorkLoad(int connections) { return ONE_CONNECTION_WORKLOAD_VALUE * connections; }

std::string NodeModule::EnumNodeTypeToString(rpc::NodeType type) {
    switch (type) {
    case rpc::ST_MASTER:
        return "master";
    case rpc::ST_WEB:
        return "web";
    case rpc::ST_WORLD:
        return "world";
    case rpc::ST_DB_PROXY:
        return "db_proxy";
    case rpc::ST_PROXY:
        return "proxy";
    case rpc::ST_PLAYER:
        return "player";
    case rpc::ST_GAME_MGR:
        return "game_mgr";
    case rpc::ST_GAME:
        return "game";
    case rpc::ST_MICRO:
        return "micro";
    case rpc::ST_CDN:
        return "cdn";
    case rpc::ST_BACKSTAGE:
        return "backstage";
    }
    return std::string();
}

rpc::NodeType NodeModule::StringNodeTypeToEnum(const std::string &type) {
    if (type == "master")
        return rpc::ST_MASTER;
    else if (type == "web")
        return rpc::ST_WEB;
    else if (type == "world")
        return rpc::ST_WORLD;
    else if (type == "db_proxy")
        return rpc::ST_DB_PROXY;
    else if (type == "proxy")
        return rpc::ST_PROXY;
    else if (type == "player")
        return rpc::ST_PLAYER;
    else if (type == "game_mgr")
        return rpc::ST_GAME_MGR;
    else if (type == "game")
        return rpc::ST_GAME;
    else if (type == "micro")
        return rpc::ST_MICRO;
    else if (type == "cdn")
        return rpc::ST_CDN;
    else if (type == "backstage")
        return rpc::ST_BACKSTAGE;
    return rpc::ST_NONE;
}

bool NodeModule::Listen() {
    m_net_->AddReceiveCallBack(this, &NodeModule::InvalidMessage);
    m_net_->AddEventCallBack(this, &NodeModule::OnServerSocketEvent);

    node_info_.info->set_id(pm_->GetArg("id=", ARG_DEFAULT_ID));
    node_info_.info->set_type(pm_->GetAppType());
    node_info_.info->set_port(pm_->GetArg("port=", ARG_DEFAULT_PORT));

    node_info_.info->set_name(pm_->GetAppName());
    node_info_.info->set_ip(pm_->GetArg("ip=", ARG_DEFAULT_IP));
    node_info_.info->set_public_ip(pm_->GetArg("public_ip=", ARG_DEFAULT_PUBLIC_IP));
    node_info_.info->set_area(pm_->GetArea());
    node_info_.info->set_update_time(SquickGetTimeS());
    node_info_.info->set_max_online(pm_->GetArg("max_conn=", ARG_DEFAULT_MAX_CONNECTION));
    node_info_.info->set_cpu_count(pm_->GetArg("cpu_count=", ARG_DEFAULT_CPU_COUNT));

    int nRet = m_net_->Listen(node_info_.info->max_online(), node_info_.info->port(), node_info_.info->cpu_count(),
                              pm_->GetArg("net_server_buffer=", ARG_DEFAULT_NET_SERVER_BUFFER_SIZE));
    LOG_INFO("Node Listen at 0.0.0.0:%v Name :", node_info_.info->port(), node_info_.info->name());

    if (nRet < 0) {
        LOG_ERROR("Cannot init server net, Port<%v>", node_info_.info->port());
        SQUICK_ASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
        exit(0);
    }
    return true;
}

// Add upper server
bool NodeModule::AddSubscribeNode(const vector<int> &types) {
    m_net_client_->AddEventCallBack(rpc::ST_MASTER, this, &NodeModule::OnClientSocketEvent);
    m_net_client_->AddReceiveCallBack(rpc::ST_MASTER, rpc::IdNNtfNodeAdd, this, &NodeModule::OnNNtfNodeAdd);
    m_net_client_->AddReceiveCallBack(rpc::ST_MASTER, rpc::IdNNtfNodeRemove, this, &NodeModule::OnNNtfNodeRemove);
    m_net_client_->AddReceiveCallBack(rpc::ST_MASTER, rpc::IdNAckNodeRegister, this, &NodeModule::OnNAckNodeRegister);
    m_net_client_->AddReceiveCallBack(rpc::ST_MASTER, rpc::IdNReqReload, this, &NodeModule::OnNReqReload);

    bool ret = false;
    node_info_.listen_types = types;
    ConnectData s;
    s.id = DEFAULT_MASTER_ID;
    s.type = StringNodeTypeToEnum("master");
    s.ip = pm_->GetArg("master_ip=", "127.0.0.1");
    s.port = pm_->GetArg("master_port=", 10001);
    s.name = "master";
    s.buffer_size = pm_->GetArg("net_client_buffer=", ARG_DEFAULT_NET_CLIENT_BUFFER_SIZE);
    LOG_INFO("Node Connect to %v %v:%v cur_area<%v>", s.name, s.ip, s.port, pm_->GetArg("area=", 0));
    m_net_client_->AddNode(s);
    return true;
}

void NodeModule::InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { printf("Net || umsg_id=%d\n", msg_id); }

// Add upper server
void NodeModule::OnDynamicServerAdd(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    uint64_t uid;
    rpc::NNtfNodeAdd ntf;
    if (!INetModule::ReceivePB(msg_id, msg, len, ntf, uid)) {
        return;
    }
    for (int i = 0; i < ntf.node_list().size(); ++i) {
        const rpc::Node &sd = ntf.node_list(i);
        // type
        ConnectData s;
        s.id = sd.id();
        s.ip = sd.ip();
        s.port = sd.port();
        s.name = sd.name();
        s.type = sd.type();
        m_net_client_->AddNode(s);
    }
}

void NodeModule::OnReloadConfig(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void NodeModule::OnReloadLua(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

// Report to upper server
void NodeModule::UpdateState() {
    node_info_.info->set_update_time(SquickGetTimeS());
    node_info_.info->set_workload(workload_);
    node_info_.info->set_connections(m_net_->GetNet()->GetConnections());
    node_info_.info->set_net_client_connections(m_net_client_->GetConnections());
    // Update status to master
    if (pm_->GetAppType() != rpc::ST_MASTER) {
        rpc::NNtfNodeReport req;
        req.set_id(pm_->GetAppID());
        auto s = req.add_list();
        *s = *node_info_.info.get();
        m_net_client_->SendPBByID(DEFAULT_MASTER_ID, rpc::IdNNtfNodeReport, req);
    }
}

// 作为服务的监听socket状态事件
void NodeModule::OnServerSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        LOG_INFO("Net Server: SQUICK_NET_EVENT_EOF Connection closed, sock<%v>", sock);
        OnClientDisconnected(sock);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        LOG_INFO("Net Server: SQUICK_NET_EVENT_ERROR Got an error on the connection, sock<%v>", sock);
        OnClientDisconnected(sock);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        LOG_INFO("Net Server: SQUICK_NET_EVENT_TIMEOUT read timeout, sock<%v>", sock);
        OnClientDisconnected(sock);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        LOG_INFO("Net Server: SQUICK_NET_EVENT_CONNECTED connected success, sock<%v>", sock);
        OnClientConnected(sock);
    }
}

void NodeModule::OnClientConnected(socket_t sock){};
void NodeModule::OnClientDisconnected(socket_t sock){};

// 作为客户端连接socket事件
void NodeModule::OnClientSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        LOG_WARN("Net Client: SQUICK_NET_EVENT_EOF, sock<%v>", sock);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        LOG_ERROR("Net Client: SQUICK_NET_EVENT_ERROR, sock<%v>", sock);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        LOG_ERROR("Net Client: SQUICK_NET_EVENT_TIMEOUT, sock<%v>", sock);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        LOG_INFO("Net Client: SQUICK_NET_EVENT_CONNECTED connected success, sock<%v>", sock);
        OnUpperNodeConnected(pNet);
    }
}

void NodeModule::OnUpperNodeConnected(INet *pNet) {
    std::shared_ptr<ConnectData> ts = m_net_client_->GetServerNetInfo(pNet);
    if (ts == nullptr) {
        ostringstream msg;
        LOG_ERROR("OnUpperNodeConnected: %v", "Cannot find server info");
        return;
    }
    ts->state = ConnectDataState::NORMAL;

    // target type only master can register
    if (ts->type != rpc::ST_MASTER)
        return;

    rpc::NReqNodeRegister req;
    *req.mutable_node() = *node_info_.info.get();

    for (auto type : node_info_.listen_types) {
        req.add_listen_type_list(type);
    }

    m_net_client_->SendPBByID(ts->id, rpc::IdNReqNodeRegister, req);
    LOG_INFO("Register node <%v>", ts->name);
}

void NodeModule::OnNAckNodeRegister(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    uint64_t uid;
    rpc::NAckNodeRegister ack;
    if (!m_net_->ReceivePB(msg_id, msg, len, ack, uid)) {
        return;
    }

    if (ack.code() == 0) {
        AddNodes(ack.node_add_list());
    } else {
        LOG_ERROR("Node Register faild! sock<%v>", sock);
    }
}

// Add node ntf
void NodeModule::OnNNtfNodeAdd(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    uint64_t uid;
    rpc::NNtfNodeAdd ntf;
    if (!m_net_->ReceivePB(msg_id, msg, len, ntf, uid)) {
        return;
    }
    AddNodes(ntf.node_list(), true);
}

bool NodeModule::AddNodes(const google::protobuf::RepeatedPtrField<rpc::Node> &list, bool from_ntf) {
    for (const auto &n : list) {
        LOG_INFO("Add node from master, is_ntf<%v> added:", from_ntf, n.name());
        ConnectData s;
        s.id = n.id();
        s.ip = n.ip();
        s.port = n.port();
        s.name = n.name();
        s.type = n.type();
        m_net_client_->AddNode(s);
    }
    return true;
}

// Reload config or lua script
void NodeModule::OnNReqReload(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    uint64_t uid;
    rpc::NReqReload req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, uid)) {
        return;
    }

    LOG_INFO("Reload type: %v", (int)req.type());
}

// Add node ntf
void NodeModule::OnNNtfNodeRemove(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

bool NodeModule::RemoveNodes() { return true; }

ServerInfo &NodeModule::GetNodeInfo() { return node_info_; }
