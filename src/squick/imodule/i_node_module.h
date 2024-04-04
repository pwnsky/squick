// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-04-11
// Description: Server node base module
#pragma once
#include <squick/core/base.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>

#define DEFAULT_NODE_MASTER_ID 1
#define DEFAULT_NODE_CPUT_COUNT 100
#define DEFAULT_NODE_MAX_SERVER_CONNECTION 100000

class INodeBaseModule : public IModule {
  public:
    virtual bool Awake() final { return true; }

    virtual bool Start() override final {
        m_net_ = pm_->FindModule<INetModule>();
        m_log_ = pm_->FindModule<ILogModule>();
        m_net_client_ = pm_->FindModule<INetClientModule>();
        is_update_ = true;

        pm_->SetAppType(StringNodeTypeToEnum(pm_->GetArg("type=", "proxy")));
        pm_->SetArea(pm_->GetArg("area=", 0));

        return true;
    }

    virtual bool Update() override final {
        // calc work load
        CalcWorkLoad();
        if (last_report_time_ + 3 > pm_->GetNowTime()) {
            return true;
        }
        if (last_report_time_ > 0) {
            UpdateState();
        }
        last_report_time_ = pm_->GetNowTime();


        return true;
    }

    inline void CalcWorkLoad() {
        // handle time * 100 + m_net connections + m_net_client connections
        time_t now_time = SquickGetTimeMS();
        // The first update will int overflow, but who care?
        workload_ = (now_time - last_update_time_) * 100;
        last_update_time_ = now_time;
    }

    static std::string EnumNodeTypeToString(ServerType type)
    {
        switch (type) {
        case ServerType::ST_MASTER: return "master";
        case ServerType::ST_LOGIN: return "login";
        case ServerType::ST_WORLD: return "world";
        case ServerType::ST_DB_PROXY: return "db_proxy";
        case ServerType::ST_PROXY: return "proxy";
        case ServerType::ST_PLAYER: return "player";
        case ServerType::ST_GAME_MGR: return "game_mgr";
        case ServerType::ST_GAME: return "game";
        case ServerType::ST_MICRO: return "micro";
        case ServerType::ST_CDN: return "cdn";
        case ServerType::ST_ROBOT: return "robot";
        }
        return "";
    }

    static ServerType StringNodeTypeToEnum(const std::string& type)
    {
        if (type == "master") return ServerType::ST_MASTER;
        else if (type == "login") return ServerType::ST_LOGIN;
        else if (type == "world") return ServerType::ST_WORLD;
        else if (type == "db_proxy") return ServerType::ST_DB_PROXY;
        else if (type == "proxy") return ServerType::ST_PROXY;
        else if (type == "player") return ServerType::ST_PLAYER;
        else if (type == "game_mgr") return ServerType::ST_GAME_MGR;
        else if (type == "game") return ServerType::ST_GAME;
        else if (type == "micro") return ServerType::ST_MICRO;
        else if (type == "cdn") return ServerType::ST_CDN;
        else if (type == "robot") return ServerType::ST_ROBOT;
        return ServerType::ST_NONE;
    }

    virtual bool Listen() {        
        m_net_->AddReceiveCallBack(this, &INodeBaseModule::InvalidMessage);
        m_net_->AddEventCallBack(this, &INodeBaseModule::OnServerSocketEvent);
        m_net_->ExpandBufferSize();

        node_info_.info->set_id(pm_->GetArg("id=", 0));
        std::string name = pm_->GetArg("type=", "proxy") + pm_->GetArg("id=", "0");
        node_info_.info->set_type(pm_->GetAppType());
        node_info_.info->set_port(pm_->GetArg("port=", 10000));

        node_info_.info->set_name(name);
        node_info_.info->set_ip(pm_->GetArg("ip=", "127.0.0.1"));
        node_info_.info->set_public_ip(pm_->GetArg("public_ip=", "127.0.0.1"));
        node_info_.info->set_area(pm_->GetArea());
        node_info_.info->set_update_time(SquickGetTimeS());
        node_info_.info->set_max_online(DEFAULT_NODE_MAX_SERVER_CONNECTION);
        node_info_.info->set_cpu_count(DEFAULT_NODE_CPUT_COUNT);

        

        //servers_[pm_->GetAppID()] = s;

        int nRet = m_net_->Startialization(node_info_.info->max_online(), node_info_.info->port(), node_info_.info->cpu_count());

        std::ostringstream log;
        log << "Node Listen at 0.0.0.0:" << node_info_.info->port() << " Name: " << node_info_.info->name();
        m_log_->LogDebug(NULL_OBJECT, log, __FUNCTION__, __LINE__);

        if (nRet < 0) {
            std::ostringstream strLog;
            strLog << "Cannot init server net, Port = " << node_info_.info->port();
            m_log_->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
            SQUICK_ASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
            exit(0);
        }
        return true;
    }

    // Add upper server
    bool AddNodesByType(const vector<int> &types) {
        m_net_client_->AddEventCallBack(ST_MASTER, this, &INodeBaseModule::OnClientSocketEvent);
        m_net_client_->AddReceiveCallBack(ST_MASTER, rpc::NMasterRPC::NNTF_NODE_ADD, this, &INodeBaseModule::OnNNtfNodeAdd);
        m_net_client_->AddReceiveCallBack(ST_MASTER, rpc::NMasterRPC::NNTF_NODE_REMOVE, this, &INodeBaseModule::OnNNtfNodeRemove);
        m_net_client_->AddReceiveCallBack(ST_MASTER, rpc::NMasterRPC::NACK_NODE_REGISTER, this, &INodeBaseModule::OnNAckNodeRegister);
        m_net_client_->ExpandBufferSize();
        bool ret = false;
        node_info_.listen_types = types;
        ConnectData s;
        s.id = DEFAULT_NODE_MASTER_ID;
        s.type = StringNodeTypeToEnum("master");
        s.ip = pm_->GetArg("master_ip=", "127.0.0.1");
        s.port = pm_->GetArg("master_port=", 10001);
        s.name = "master";
        std::ostringstream log;
        log << "Node Connect to " << s.name << " host " << s.ip << ":" << s.port << " cur_area: " << pm_->GetArg("area=", 0) << std::endl;
        m_log_->LogDebug(NULL_OBJECT, log, __FUNCTION__, __LINE__);
        m_net_client_->AddNode(s);
        return true;
    }

    void InvalidMessage(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) { printf("Net || umsg_id=%d\n", msg_id); }

    // Add upper server
    void OnDynamicServerAdd(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        uint64_t uid;
        rpc::NNtfNodeAdd ntf;
        if (!INetModule::ReceivePB(msg_id, msg, len, ntf, uid)) {
            return;
        }
        for (int i = 0; i < ntf.node_list().size(); ++i) {
            const rpc::Server &sd = ntf.node_list(i);
            // type
            ConnectData s;
            s.id = sd.id();
            s.ip = sd.ip();
            s.port = sd.port();
            s.name = sd.name();
            s.type = (ServerType)sd.type();
            m_net_client_->AddNode(s);
        }
    }
    //////////////////////////// PRIVATE ////////////////////////////
  private:
    // 热加载配置文件
    virtual void OnReloadConfig(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    
    }
    // 热加载Lua脚本
    virtual void OnReloadLua(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {

    }

    // Report to upper server
    void UpdateState() {

        node_info_.info->set_update_time(SquickGetTimeS());
        node_info_.info->set_workload(workload_);
        // Update status to master
        if (pm_->GetAppType() != ST_MASTER) {
            rpc::NNtfNodeReport req;
            req.set_id(pm_->GetAppID());
            auto s = req.add_list();
            *s = *node_info_.info.get();
            m_net_client_->SendPBByID(DEFAULT_NODE_MASTER_ID, rpc::NMasterRPC::NNTF_NODE_REPORT, req);
        }
    }

    // 作为服务的监听socket状态事件
    void OnServerSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
        if (eEvent & SQUICK_NET_EVENT_EOF) {
            m_log_->LogInfo(Guid(0, sock), "Net Server: SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
            OnClientDisconnected(sock);
        } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
            m_log_->LogInfo(Guid(0, sock), "Net Server: SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
            OnClientDisconnected(sock);
        } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
            m_log_->LogInfo(Guid(0, sock), "Net Server: SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
            OnClientDisconnected(sock);
        } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
            m_log_->LogInfo(Guid(0, sock), "Net Server: SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
            OnClientConnected(sock);
        }
    }

    virtual void OnClientConnected(socket_t sock) {};
    virtual void OnClientDisconnected(socket_t sock) {};

    // 作为客户端连接socket事件
    void OnClientSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet* pNet) {
        if (eEvent & SQUICK_NET_EVENT_EOF) {
            m_log_->LogWarning(Guid(0, sock), "Net Client: SQUICK_NET_EVENT_EOF", __FUNCTION__, __LINE__);
        }
        else if (eEvent & SQUICK_NET_EVENT_ERROR) {
            m_log_->LogError(Guid(0, sock), "Net Client: SQUICK_NET_EVENT_ERROR", __FUNCTION__, __LINE__);
        }
        else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
            m_log_->LogError(Guid(0, sock), "Net Client: SQUICK_NET_EVENT_TIMEOUT", __FUNCTION__, __LINE__);
        }
        else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
            m_log_->LogInfo(Guid(0, sock), "Net Client: SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
            
            OnUpperNodeConnected(pNet);
        }
    }

    void OnUpperNodeConnected(INet* pNet) {
        std::shared_ptr<ConnectData> ts = m_net_client_->GetServerNetInfo(pNet);
        if (ts == nullptr) {
            ostringstream msg;
            msg << " Cannot find server info ";
            m_log_->LogWarning(msg, __FUNCTION__, __LINE__);
            return;
        }
        ts->state = ConnectDataState::NORMAL;

        // target type only master can register
        if (ts->type != ST_MASTER) return;
        
        rpc::NReqNodeRegister req;
        *req.mutable_node() = *node_info_.info.get();
        
        for (auto type : node_info_.listen_types) {
            req.add_listen_type_list(type);
        }

        m_net_client_->SendPBByID(ts->id, rpc::NMasterRPC::NREQ_NODE_REGISTER, req);
        m_log_->LogInfo(Guid(0, pm_->GetAppID()), ts->name, "Register");
    }
    
    void OnNAckNodeRegister(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
        uint64_t uid;
        rpc::NAckNodeRegister ack;
        if (!m_net_->ReceivePB(msg_id, msg, len, ack, uid)) {
            return;
        }

        if (ack.code() == 0) {
            AddNodes(ack.node_add_list());
        }
        else {
            m_log_->LogError(Guid(0, pm_->GetAppID()), "Register faild!");
        }
    }

    // Add node ntf
    void OnNNtfNodeAdd(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
        uint64_t uid;
        rpc::NNtfNodeAdd ntf;
        if (!m_net_->ReceivePB(msg_id, msg, len, ntf, uid)) {
            return;
        }
        AddNodes(ntf.node_list(), true);
    }

    bool AddNodes(const google::protobuf::RepeatedPtrField<rpc::Server>& list, bool from_ntf = false) {
        for (const auto &n : list) {
            dout << "Add node from master, is_ntf: " << from_ntf << " cmd, current: " << node_info_.info->name() << " add " << n.name() << endl;
            ConnectData s;
            s.id = n.id();
            s.ip = n.ip();
            s.port = n.port();
            s.name = n.name();
            s.type = (ServerType)n.type();
            m_net_client_->AddNode(s);
        }
        return true;
    }

    // Add node ntf
    void OnNNtfNodeRemove(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {

    }

    bool RemoveNodes() {
        return true;
    }

    public:
    ILogModule *m_log_;
    INetModule *m_net_;
    INetClientModule *m_net_client_;
    ServerInfo node_info_;

  private:
    time_t last_report_time_ = 0;
    time_t last_update_time_ = 0;
    int workload_;

  protected:
};
