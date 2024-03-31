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

#define DEFAULT_MASTER_ID 1

class INodeBaseModule : public IModule {
  public:
    virtual bool Awake() final { return true; }
    virtual bool Start() override final {
        m_net_ = pm_->FindModule<INetModule>();
        m_log_ = pm_->FindModule<ILogModule>();
        m_element_ = pm_->FindModule<IElementModule>();
        m_class_ = pm_->FindModule<IClassModule>();
        m_net_client_ = pm_->FindModule<INetClientModule>();
        is_update_ = true;

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
        return "";
    }

    static ServerType StringNodeTypeToEnum(const std::string& type)
    {
        if (type == "master") return ServerType::ST_MASTER;
        else if (type == "login") return ServerType::ST_LOGIN;
        else if (type == "world") return ServerType::ST_WORLD;
        else if (type == "db_proxy") return ServerType::ST_DB_PROXY;
        else if (type == "proxy") return ServerType::ST_PROXY;
        else if (type == "lobby") return ServerType::ST_LOBBY;
        else if (type == "game_mgr") return ServerType::ST_GAME_MGR;
        else if (type == "game") return ServerType::ST_GAME;
        else if (type == "micro") return ServerType::ST_MICRO;
        else if (type == "cdn") return ServerType::ST_CDN;
        else if (type == "robot") return ServerType::ST_ROBOT;
        return ServerType::ST_NONE;
    }

    virtual bool Listen() {
        m_net_->AddReceiveCallBack(rpc::ServerRPC::REQ_REGISTER, this, &INodeBaseModule::OnReqRegister);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::REQ_UNREGISTER, this, &INodeBaseModule::OnServerUnRegistered);
        
        m_net_->AddReceiveCallBack(this, &INodeBaseModule::InvalidMessage);
        m_net_->AddEventCallBack(this, &INodeBaseModule::OnServerSocketEvent);
        m_net_->ExpandBufferSize();

        ServerInfo s;
        s.info->set_id(pm_->GetArg("id=", 0));
        std::string name = pm_->GetArg("type=", "proxy") + pm_->GetArg("id=", "0");
        s.info->set_type(StringNodeTypeToEnum(pm_->GetArg("type=", "proxy")));
        s.info->set_port(pm_->GetArg("port=", 10000));

        s.info->set_name(name);
        s.info->set_ip(pm_->GetArg("ip=", "127.0.0.1"));
        s.info->set_public_ip(pm_->GetArg("public_ip=", "127.0.0.1"));
        s.info->set_area(pm_->GetArg("area=", 0));
        s.info->set_update_time(SquickGetTimeS());
        s.info->set_max_online(10000);
        s.info->set_cpu_count(8);
        pm_->SetAppType(s.info->type());
        pm_->SetArea(s.info->area());

        servers_[pm_->GetAppID()] = s;

        int nRet = m_net_->Startialization(s.info->max_online(), s.info->port(), s.info->cpu_count());

        std::ostringstream log;
        log << "Node Listen at 0.0.0.0:" << s.info->port() << " Name: " << s.info->name();
        m_log_->LogDebug(NULL_OBJECT, log, __FUNCTION__, __LINE__);

        if (nRet < 0) {
            std::ostringstream strLog;
            strLog << "Cannot init server net, Port = " << s.info->port();
            m_log_->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
            SQUICK_ASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
            exit(0);
        }
        return true;
    }

    // Add upper server
    bool ConnectToMaster() {
        m_net_client_->AddEventCallBack(ST_MASTER, this, &INodeBaseModule::OnClientSocketEvent);
        //m_net_client_->AddReceiveCallBack(ST_MASTER, rpc::ServerRPC::SERVER_ADD, this, &INodeBaseModule::OnDynamicServerAdd);
        m_net_client_->AddReceiveCallBack(ST_MASTER, rpc::ServerRPC::ACK_REGISTER, this, &INodeBaseModule::OnAckRegister);
        m_net_client_->ExpandBufferSize();
        bool ret = false;

        // 增加同一区服的，如果是Master或Login，不用校验区服
        
        ServerInfo info;
        info.status = ServerInfo::Status::Connecting;
        info.info->set_port(pm_->GetArg("master_port=", 10001));
        info.info->set_name("master");
        info.info->set_ip(pm_->GetArg("master_ip=", "127.0.0.1"));
        info.info->set_id(DEFAULT_MASTER_ID);
        servers_[DEFAULT_MASTER_ID] = info;
        ConnectData s;
        s.id = DEFAULT_MASTER_ID;
        s.type = StringNodeTypeToEnum("master");
        s.ip = info.info->ip();
        s.port = info.info->port();
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
        string guid;
        rpc::ServerList req;
        if (!INetModule::ReceivePB(msg_id, msg, len, req, guid)) {
            return;
        }
        for (int i = 0; i < req.list_size(); ++i) {
            const rpc::Server &sd = req.list(i);
            // type
            ConnectData s;
            s.id = sd.id();
            s.ip = sd.ip();
            s.port = sd.port();
            s.name = sd.name();
            s.work_load = sd.cpu_count();
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
        rpc::ReqReport req;
        req.set_id(pm_->GetAppID());
        auto iter = servers_.find(pm_->GetAppID());
        if (iter == servers_.end()) {
            m_log_->LogDebug("No self node info");
            return;
        }
        iter->second.info->set_update_time(SquickGetTimeS());
        iter->second.info->set_workload(workload_);
        // Update status to master
        if (pm_->GetAppType() != ST_MASTER) {
            rpc::ReqReport req;
            auto s = req.add_list();
            *s = *iter->second.info;
            m_net_client_->SendToServerByPB(DEFAULT_MASTER_ID, rpc::ServerRPC::REQ_REPORT, req);
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
            ReqRegister(pNet);
        }
    }

    // 向连接的服务注册自己
    void ReqRegister(INet* pNet) {
        rpc::ReqRegisterServer req;
        req.set_id(pm_->GetAppID());
        std::shared_ptr<ConnectData> ts = m_net_client_->GetServerNetInfo(pNet);
        if (ts == nullptr) {
            ostringstream msg;
            msg << " Cannot find server info ";
            m_log_->LogWarning(msg, __FUNCTION__, __LINE__);
            return;
        }
        auto s = req.add_list();
        *s = *servers_[pm_->GetAppID()].info.get();
        m_net_client_->SendToServerByPB(ts->id, rpc::ServerRPC::REQ_REGISTER, req);
        m_log_->LogInfo(Guid(0, pm_->GetAppID()), ts->name, "Register");
    }

    virtual void OnReqRegister(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        string nPlayerID;
        rpc::ReqRegisterServer req;
        if (!m_net_->ReceivePB(msg_id, msg, len, req, nPlayerID)) {
            return;
        }
        rpc::AckRegisterServer ack;
        do {
            auto& cs = servers_[pm_->GetAppID()];
            ack.set_code(0);
            for (auto s : req.list()) {
                if (s.id() == pm_->GetAppID()) {
                    continue;
                }
                if (s.id() == req.id()) {

                }
                ServerInfo info;
                info.fd = sock;
                *info.info = s;
                servers_[s.id()] = info;
            }

            auto iter = servers_.find(req.id());
            if (iter != servers_.end()) {
                iter->second.status = ServerInfo::Status::Connected;
            }
            
            int area = servers_[req.id()].info->area();
            for (auto sv : servers_) {
                if (area == 0 || sv.second.info->area() == area || sv.second.info->area() == 0) {
                    auto s = ack.add_list();
                    *s = *sv.second.info.get();
                }
            }
        } while (false);
        m_net_->SendMsgPB(rpc::ACK_REGISTER, ack, sock);
    }
    
    // 注册响应
    virtual void OnAckRegister(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
        string guid;
        rpc::AckRegisterServer ack;
        if (!m_net_->ReceivePB(msg_id, msg, len, ack, guid)) {
            return;
        }

        if (ack.code() == 0) {
            for (auto s : ack.list()) {
                if (s.id() == pm_->GetAppID()) { // 排除自己
                    continue;
                }
                auto iter = servers_.find(s.id());
                if (iter == servers_.end()) {
                    ServerInfo info;
                    info.fd = 0;
                    info.status = ServerInfo::Status::Unknowing;
                    *info.info = s;
                    servers_[s.id()] = info;
                }
                else {
                    iter->second.status = ServerInfo::Status::Connected;
                    *iter->second.info = s;
                }
            }
        }
        else {
            dout << "注册失败!";
        }
    }

    virtual void OnServerUnRegistered(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        string nPlayerID;
        rpc::ServerList sl;
        if (!m_net_->ReceivePB(msg_id, msg, len, sl, nPlayerID)) {
            return;
        }

        for (int i = 0; i < sl.list_size(); ++i) {
            const rpc::Server &s = sl.list(i);
            int id = s.id();
            auto iter = servers_.find(id);
            if (iter != servers_.end()) {
                servers_.erase(iter);
            }
            m_log_->LogInfo(Guid(0, s.id()), s.name(), " UnRegistered");
        }
    }
    
    public:
        virtual int GetLoadBanlanceNode(ServerType type) {
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

    public:

    // 服务表
    map<int, ServerInfo> servers_;
    
    IElementModule *m_element_;
    IClassModule *m_class_;
    ILogModule *m_log_;
    INetModule *m_net_;
    INetClientModule *m_net_client_;

  private:
    time_t last_report_time_ = 0;
    time_t last_update_time_ = 0;
    int workload_;

  protected:
};