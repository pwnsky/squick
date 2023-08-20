// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2023-04-11
// Description: Server node base module
#pragma once
#include <squick/core/base.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>

class INodeBaseModule : public IModule {
  public:
      virtual bool Awake() final { return true; }
    virtual bool Start() override final {
        m_net_ = pm_->FindModule<INetModule>();
        m_kernel_ = pm_->FindModule<IKernelModule>();
        m_log_ = pm_->FindModule<ILogModule>();
        m_element_ = pm_->FindModule<IElementModule>();
        m_class_ = pm_->FindModule<IClassModule>();
        m_net_client_ = pm_->FindModule<INetClientModule>();
        m_thread_pool_ = pm_->FindModule<IThreadPoolModule>();
        
        is_update_ = true;

        return true;
    }

    virtual bool Update() override final {
        if (last_report_time_ + 15 > pm_->GetNowTime()) {
            return true;
        }
        last_report_time_ = pm_->GetNowTime();
        ReqServerReport();
        return true;
    }

    bool Listen() {
        m_net_->AddReceiveCallBack(rpc::ServerRPC::REQ_REGISTER, this, &INodeBaseModule::OnReqRegister);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::REQ_UNREGISTER, this, &INodeBaseModule::OnServerUnRegistered);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::REQ_REPORT, this, &INodeBaseModule::OnReqServerReport);


        // Player action
        m_net_->AddReceiveCallBack(rpc::ServerRPC::PLAYER_ENETER, this, &INodeBaseModule::OnPlayerEnter);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::PLAYER_LEAVE, this, &INodeBaseModule::OnPlayerLeave);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::PLAYER_OFFLINE, this, &INodeBaseModule::OnPlayerOffline);


        m_net_->AddReceiveCallBack(rpc::ServerRPC::SERVER_HEARTBEAT, this, &INodeBaseModule::OnHeartBeat);
        m_net_->AddReceiveCallBack(this, &INodeBaseModule::InvalidMessage);
        

        m_net_->AddEventCallBack(this, &INodeBaseModule::OnServerSocketEvent);
        m_net_->ExpandBufferSize();

        std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
        if (xLogicClass) {
            const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
            for (int i = 0; i < strIdList.size(); ++i) {
                const std::string &strId = strIdList[i];
                const int id = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
                if (pm_->GetAppID() == id) {
                    ServerInfo s;
                    s.info->set_id(id);
                    s.info->set_key(m_element_->GetPropertyString(strId, excel::Server::Key()));
                    s.info->set_type(m_element_->GetPropertyInt32(strId, excel::Server::Type()));
                    s.info->set_port(m_element_->GetPropertyInt32(strId, excel::Server::Port()));
                    s.info->set_max_online(m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline()));
                    s.info->set_cpu_count(m_element_->GetPropertyInt32(strId, excel::Server::CpuCount()));
                    s.info->set_name(m_element_->GetPropertyString(strId, excel::Server::ID()));
                    s.info->set_ip(m_element_->GetPropertyString(strId, excel::Server::IP()));
                    s.info->set_public_ip(m_element_->GetPropertyString(strId, excel::Server::PublicIP()));
                    s.info->set_area(m_element_->GetPropertyInt32(strId, excel::Server::Area()));
                    s.info->set_cpu_count(m_element_->GetPropertyInt32(strId, excel::Server::CpuCount()));
                    s.type = ServerInfo::Type::Self;
                    s.info->set_update_time(SquickGetTimeS());
                    pm_->SetAppType(s.info->type());
                    pm_->SetArea(s.info->area());
                    servers_[pm_->GetAppID()] = s;

                    int nRet = m_net_->Startialization(s.info->max_online(), s.info->port(), s.info->cpu_count());
                    if (nRet < 0) {
                        std::ostringstream strLog;
                        strLog << "Cannot init server net, Port = " << s.info->port();
                        m_log_->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                        SQUICK_ASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
                        exit(0);
                    }
                }
            }
        }
        return true;
    }

    // Add upper server
    bool AddServer(ServerType type) {
        m_net_client_->AddEventCallBack(type, this, &INodeBaseModule::OnClientSocketEvent);
        m_net_client_->AddReceiveCallBack(type, rpc::ServerRPC::SERVER_ADD, this, &INodeBaseModule::OnDynamicServerAdd);
        m_net_client_->AddReceiveCallBack(type, rpc::ServerRPC::ACK_REGISTER, this, &INodeBaseModule::OnAckRegister);
        m_net_client_->AddReceiveCallBack(type, rpc::ServerRPC::ACK_REPORT, this, &INodeBaseModule::OnAckServerReport);
        m_net_client_->ExpandBufferSize();

        std::shared_ptr<IClass> config = m_class_->GetElement(excel::Server::ThisName());
        if (config) {
            const std::vector<std::string>& list = config->GetIDList();
            const int cur_area = pm_->GetArea();
            for (auto k : list) {
                const int server_type = m_element_->GetPropertyInt32(k, excel::Server::Type());
                const int area = m_element_->GetPropertyInt32(k, excel::Server::Area());
                if (server_type == type && cur_area == area) { // 同一区服注册
                    int id = m_element_->GetPropertyInt32(k, excel::Server::ServerID());
                    ServerInfo info;
                    
                    info.type = ServerInfo::Type::Parrent; // 标识该节点为父节点
                    info.status = ServerInfo::Status::Connecting;
                    info.info->set_port(m_element_->GetPropertyInt32(k, excel::Server::Port()));
                    info.info->set_name(m_element_->GetPropertyString(k, excel::Server::ID()));
                    info.info->set_ip(m_element_->GetPropertyString(k, excel::Server::IP()));
                    info.info->set_id(id);
                    servers_[id] = info;

                    ConnectData s;
                    s.id = id;
                    s.type = (ServerType)server_type;
                    s.ip = info.info->ip();
                    s.port = info.info->port();
                    s.name = k;
                    m_net_client_->AddServer(s);
                    return true;
                }
            }
        }
        return false;
    }


    void OnHeartBeat(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
        
    }

    void InvalidMessage(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) { printf("Net || umsg_id=%d\n", msg_id); }


    void LogServerInfo(const std::string &strServerInfo) { m_log_->LogInfo(Guid(), strServerInfo, ""); }

    // Add upper server
    void OnDynamicServerAdd(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        Guid guid;
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
            m_net_client_->AddServer(s);
        }
    }
    
    

    // 发送消息给玩家
    virtual void SendPBToPlayer(const uint16_t msg_id, google::protobuf::Message &msg, const Guid &player) {}

    virtual void SendToPlayer(const uint16_t msg_id, const std::string &msg, const Guid &self) {}

    // 根据服务id发消息给其他服务器，该方式通过代理服务器进行中转
    virtual void SendPBToServer(const uint16_t msg_id, google::protobuf::Message &msg, int server_id) {}

    virtual void SendToServer(const uint16_t msg_id, const std::string &msg, int server_id) {}

    // 发送给玩家所连接相应类型的服务器
    virtual void SendPBToPlayerServer(const uint16_t msg_id, google::protobuf::Message &msg, const Guid &player, ServerType type) {}

    virtual void SendToPlayerServer(const uint16_t msg_id, const std::string &msg, const Guid &player, ServerType type) {}

    virtual void OnPlayerEnter(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        // players_[]
    }
    
    virtual void OnPlayerLeave(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

    virtual void OnPlayerOffline(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

    //////////////////////////// PRIVATE ////////////////////////////
  private:
    // 热加载配置文件
    virtual void OnReloadConfig(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    
    }
    // 热加载Lua脚本
    virtual void OnReloadLua(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {

    }

    virtual void OnServerRefreshed(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        Guid nPlayerID;
        rpc::ServerList sl;
        if (!m_net_->ReceivePB(msg_id, msg, len, sl, nPlayerID)) {
            return;
        }
        for (int i = 0; i < sl.list_size(); ++i) {
            const rpc::Server &s = sl.list(i);
            int id = s.id();
            ServerInfo d;
            d.fd = sock;
            *d.info = s;
            servers_[id] = d;
            m_log_->LogInfo(Guid(0, s.id()), s.name(), " Refreshed");
        }
    }

    // Report to upper server
    void ReqServerReport() {
        rpc::ReqReport req;
        // 更新自己的时间
        auto iter = servers_.find(pm_->GetAppID());
        iter->second.info->set_update_time(SquickGetTimeS());

        // 将下游服务和自己全部注册更新到上游
        for (auto sv : servers_) {
            if (sv.second.type == ServerInfo::Type::Child || sv.second.type == ServerInfo::Type::Self) {
                auto s = req.add_list();
                
                *s = *sv.second.info.get();
            }
        }
        for (auto sv : servers_) {
            if (sv.second.type == ServerInfo::Type::Parrent && sv.second.status == ServerInfo::Status::Connected) {
                m_net_client_->SendToServerByPB(sv.second.info->id(), rpc::REQ_REPORT, req);
            }
        }
    }

    virtual void OnReqServerReport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        Guid guid;
        rpc::ReqReport req;
        if (!INetModule::ReceivePB(msg_id, msg, len, req, guid)) {
            return;
        }
        rpc::AckRegisterServer ack;
        do {
            ack.set_code(0);
            for (auto s : req.list()) {
                if (s.id() == pm_->GetAppID()) { // 排除自己和比当前时间更新晚的
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

                // 新增
                ServerInfo info;
                info.fd = 0;
                info.status = ServerInfo::Status::Unknowing;
                info.type = ServerInfo::Type::Unknowing;
                *info.info = s;
                servers_[s.id()] = info;
            }

            // 更新下游状态
            for (auto sv : servers_) {
                auto s = ack.add_list();
                *s = *sv.second.info.get();
                
            }
        } while (false);
        m_net_->SendMsgPB(rpc::ServerRPC::ACK_REPORT, ack, sock);

    }

    virtual void OnAckServerReport(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
        Guid guid;
        rpc::AckReport ack;
        if (!m_net_->ReceivePB(msg_id, msg, len, ack, guid)) {
            return;
        }

        if (ack.code() == 0) {
            for (auto s : ack.list()) {
                if (s.id() == pm_->GetAppID()) { // 排除自己
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
                
                // 新增
                ServerInfo info;
                info.fd = 0;
                info.status = ServerInfo::Status::Unknowing;
                info.type = ServerInfo::Type::Unknowing;
                *info.info = s;
                servers_[s.id()] = info;
            }
        }
        else {
            dout << "注册失败!";
        }
    }

    // 作为服务的监听socket状态事件
    void OnServerSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
        if (eEvent & SQUICK_NET_EVENT_EOF) {
            m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
            OnClientDisconnect(sock);
        } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
            m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
            OnClientDisconnect(sock);
        } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
            m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
            OnClientDisconnect(sock);
        } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
            m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
            OnClientConnected(sock);
        }
    }

    virtual void OnClientDisconnect(socket_t sock) {};
    virtual void OnClientConnected(socket_t sock) {};

    // 作为客户端连接socket事件
    void OnClientSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet* pNet) {
        if (eEvent & SQUICK_NET_EVENT_EOF) {
            m_log_->LogWarning(Guid(0, sock), "SQUICK_NET_EVENT_EOF", __FUNCTION__, __LINE__);
        }
        else if (eEvent & SQUICK_NET_EVENT_ERROR) {
            m_log_->LogError(Guid(0, sock), "SQUICK_NET_EVENT_ERROR", __FUNCTION__, __LINE__);
        }
        else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
            m_log_->LogError(Guid(0, sock), "SQUICK_NET_EVENT_TIMEOUT", __FUNCTION__, __LINE__);
        }
        else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
            m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
            ReqRegister(pNet);
        }
    }


    // 向连接的服务注册自己
    void ReqRegister(INet* pNet) {
        rpc::ReqRegisterServer req;

        std::shared_ptr<ConnectData> ts = m_net_client_->GetServerNetInfo(pNet);
        if (ts == nullptr) {
            ostringstream msg;
            msg << " Cannot find server info ";
            m_log_->LogWarning(msg, __FUNCTION__, __LINE__);
            return;
        }
        req.set_key("no passowrd");
        // 取出即将连接服务器的密钥
        std::shared_ptr<IClass> config = m_class_->GetElement(excel::Server::ThisName());
        if (config) {
            const std::vector<std::string>& idx_list = config->GetIDList();

            for (auto& idx : idx_list) {
                int id = m_element_->GetPropertyInt32(idx, excel::Server::ServerID());
                if (id == ts->id) {
                    string key = m_element_->GetPropertyString(idx, excel::Server::Key());
                    req.set_key(key);
                    break;
                }
            }
        }

        // 将下游服务和自己全部注册到上游
        for (auto sv : servers_) {
            if (sv.second.type == ServerInfo::Type::Child || sv.second.type == ServerInfo::Type::Self) {
                auto s = req.add_list();
                *s = *sv.second.info.get();
            }
        }

        if (ts) {
            m_net_client_->SendToServerByPB(ts->id, rpc::ServerRPC::REQ_REGISTER, req);
            //dout << pm_->GetAppName() << " 请求连接 " << ts->name << "\n";
            //m_log_->LogInfo(Guid(0, pm_->GetAppID()), s->name(), "Register");
        }
    }

    virtual void OnReqRegister(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        Guid nPlayerID;
        rpc::ReqRegisterServer req;
        if (!m_net_->ReceivePB(msg_id, msg, len, req, nPlayerID)) {
            return;
        }
        rpc::AckRegisterServer ack;
        do {
            auto& cs = servers_[pm_->GetAppID()];
            // 验证key
            if (req.key() != cs.info->key()) {
                dout << " 校验key失败: req key" << req.key() << "  our key" << cs.info->key() << "\n";
                ack.set_code(1);
                break;
            }

            ack.set_code(0);
            for (auto s : req.list()) {
                if (s.id() == pm_->GetAppID()) { // 排除自己
                    continue;
                }
                ServerInfo info;
                info.fd = sock;
                *info.info = s;
                servers_[s.id()] = info;
            }

            // 将下游服务和自己全部注册到上游
            for (auto sv : servers_) {
                auto s = ack.add_list();
                *s = *sv.second.info.get();
            }
        } while (false);
        m_net_->SendMsgPB(rpc::ACK_REGISTER, ack, sock);
    }
    
    // 注册响应
    virtual void OnAckRegister(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
        Guid guid;
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
                    info.type = ServerInfo::Type::Unknowing;
                    *info.info = s;
                    servers_[s.id()] = info;
                }
                else {
                    if (iter->second.type == ServerInfo::Type::Parrent) {
                        iter->second.status = ServerInfo::Status::Connected;
                    }
                    *iter->second.info = s;
                }
                
            }
        }
        else {
            dout << "注册失败!";
        }
    }

    virtual void OnServerUnRegistered(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        Guid nPlayerID;
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

    virtual void OnReqManager(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    
    }
    
    struct PlayerProxyInfo {
        int proxy_id;
        int proxy_sock;
    };


    public:
    // 玩家表
    map<Guid, PlayerProxyInfo> players_;

    // 服务表
    map<int, ServerInfo> servers_;
    
    IElementModule *m_element_;
    IClassModule *m_class_;
    IKernelModule *m_kernel_;
    ILogModule *m_log_;
    INetModule *m_net_;
    INetClientModule *m_net_client_;
    IThreadPoolModule *m_thread_pool_;

  private:
    time_t last_report_time_ = 0;

  protected:
};