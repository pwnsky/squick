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

    //////////////////////////// PUBLIC ////////////////////////////
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
        ServerReport();
        return true;
    }

    bool Listen() {
        m_net_->AddReceiveCallBack(rpc::ServerRPC::REQ_REGISTER, this, &INodeBaseModule::OnReqRegister);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::REQ_UNREGISTER, this, &INodeBaseModule::OnServerUnRegistered);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::REQ_REPORT, this, &INodeBaseModule::OnServerReport);


        // Player action
        m_net_->AddReceiveCallBack(rpc::ServerRPC::PLAYER_ENETER, this, &INodeBaseModule::OnPlayerEnter);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::PLAYER_LEAVE, this, &INodeBaseModule::OnPlayerLeave);
        m_net_->AddReceiveCallBack(rpc::ServerRPC::PLAYER_OFFLINE, this, &INodeBaseModule::OnPlayerOffline);

        m_net_->AddEventCallBack(this, &INodeBaseModule::OnServerSocketEvent);
        m_net_->ExpandBufferSize();

        std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
        if (xLogicClass) {
            const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
            for (int i = 0; i < strIdList.size(); ++i) {
                const std::string &strId = strIdList[i];
                const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
                if (pm_->GetAppID() == serverID) {
                    ServerInfo s;
                    
                    s.info_->set_key(m_element_->GetPropertyString(strId, excel::Server::Key()));
                    s.info_->set_type(m_element_->GetPropertyInt32(strId, excel::Server::Type()));
                    s.info_->set_port(m_element_->GetPropertyInt32(strId, excel::Server::Port()));
                    s.info_->set_max_online(m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline()));
                    s.info_->set_cpu_count(m_element_->GetPropertyInt32(strId, excel::Server::CpuCount()));
                    s.info_->set_name(m_element_->GetPropertyString(strId, excel::Server::Name()));
                    s.info_->set_ip(m_element_->GetPropertyString(strId, excel::Server::IP()));
                    s.info_->set_public_ip(m_element_->GetPropertyString(strId, excel::Server::PublicIP()));
                    s.info_->set_area(m_element_->GetPropertyInt32(strId, excel::Server::Area()));
                    s.info_->set_id(m_element_->GetPropertyInt32(strId, excel::Server::ID()));
                    s.info_->set_cpu_count(m_element_->GetPropertyInt32(strId, excel::Server::CpuCount()));

                    pm_->SetAppType(s.info_->type());
                    pm_->SetArea(s.info_->type());
                    servers_[pm_->GetAppID()] = s;

                    int nRet = m_net_->Startialization(s.info_->max_online(), s.info_->port(), s.info_->cpu_count());
                    if (nRet < 0) {
                        std::ostringstream strLog;
                        strLog << "Cannot init server net, Port = " << s.info_->port();
                        m_log_->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                        SQUICK_ASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
                        exit(0);
                    }
                }
            }
        }
        return true;
    }

    void LogServerInfo(const std::string &strServerInfo) { m_log_->LogInfo(Guid(), strServerInfo, ""); }

    // Add upper server
    void OnDynamicServerAdd(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        Guid nPlayerID;
        rpc::ServerList xMsg;
        if (!INetModule::ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
            return;
        }
        for (int i = 0; i < xMsg.list_size(); ++i) {
            const rpc::Server &xData = xMsg.list(i);
            // type
            ConnectData s;
            s.id = xData.id();
            s.ip = xData.ip();
            s.port = xData.port();
            s.name = xData.name();
            s.work_load = xData.cpu_count();
            s.type = (ServerType)xData.type();
            m_net_client_->AddServer(s);
        }
    }

    // Report to upper server
    void ServerReport() {
        last_report_time_ = pm_->GetNowTime();
        std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
        if (xLogicClass) {
            const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
            for (int i = 0; i < strIdList.size(); ++i) {
                const std::string &strId = strIdList[i];

                const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
                const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
                if (pm_->GetAppID() == serverID) {
                    const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                    const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                    const std::string &name = m_element_->GetPropertyString(strId, excel::Server::ID());
                    const std::string &ip = m_element_->GetPropertyString(strId, excel::Server::IP());

                    rpc::Server s;

                    s.set_id(serverID);
                    s.set_name(strId);
                    s.set_cpu_count(0);
                    s.set_ip(ip);
                    s.set_port(nPort);
                    s.set_max_online(maxConnect);
                    s.set_state(rpc::ServerState::ServerNormal);
                    s.set_type(serverType);

                    m_net_client_->SendToAllServerByPB(ServerType::ST_WORLD, rpc::ServerRPC::REQ_REPORT, s, Guid());
                }
            }
        }
    }

    // Add upper server
    bool AddServer(ServerType type) {
        m_net_client_->AddEventCallBack(type, this, &INodeBaseModule::OnClientSocketEvent);
        m_net_client_->AddReceiveCallBack(type, rpc::SERVER_ADD, this, &INodeBaseModule::OnDynamicServerAdd);
        m_net_client_->AddReceiveCallBack(type, rpc::ACK_REGISTER, this, &INodeBaseModule::OnAckRegister);
        m_net_client_->ExpandBufferSize();

        std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
        if (xLogicClass) {
            const std::vector<std::string> &strIdList = xLogicClass->GetIDList();

            const int nCurAppID = pm_->GetAppID();
            std::vector<std::string>::const_iterator itr = std::find_if(strIdList.begin(), strIdList.end(), [&](const std::string &strConfigId) {
                return nCurAppID == m_element_->GetPropertyInt32(strConfigId, excel::Server::ServerID());
            });

            if (strIdList.end() == itr) {
                std::ostringstream strLog;
                strLog << "Cannot find current server, AppID = " << nCurAppID;
                m_log_->LogError(NULL_OBJECT, strLog, __FILE__, __LINE__);
                SQUICK_ASSERT(-1, "Cannot find current server", __FILE__, __FUNCTION__);
                exit(0);
            }

            const int nCurArea = m_element_->GetPropertyInt32(*itr, excel::Server::Area());
            for (int i = 0; i < strIdList.size(); ++i) {
                const std::string &strId = strIdList[i];

                const int server_type = m_element_->GetPropertyInt32(strId, excel::Server::Type());
                const int server_id = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
                const int area = m_element_->GetPropertyInt32(strId, excel::Server::Area());
                if (server_type == type && nCurArea == area) { // 同一区服注册
                    const int port = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                    const std::string &name = m_element_->GetPropertyString(strId, excel::Server::ID());
                    const std::string &ip = m_element_->GetPropertyString(strId, excel::Server::IP());
                    ConnectData s;
                    s.id = server_id;
                    s.type = (ServerType)server_type;
                    s.ip = ip;
                    s.port = port;
                    s.name = strId;
                    m_net_client_->AddServer(s);
                    return true;
                }
            }
        }
        return false;
    }

    virtual void OnClientDisconnect(socket_t sock) {};
    virtual void OnClientConnected(socket_t sock) {};

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
            d.fd_ = sock;
            *d.info_ = s;
            servers_[id] = d;
            m_log_->LogInfo(Guid(0, s.id()), s.name(), " Refreshed");
        }
    }

    virtual void OnServerReport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

    // 监听socket状态事件
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

    virtual void OnReqRegister(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        Guid nPlayerID;
        rpc::ReqRegisterServer req;
        if (!m_net_->ReceivePB(msg_id, msg, len, req, nPlayerID)) {
            return;
        }
        auto& cs = servers_[pm_->GetAppID()];
        // 验证key
        if (req.key() != cs.info_->key()) {
            dout << " 校验key失败: req key" << req.key() << "  our key" << cs.info_->key() << "\n";
            return;
        }
        dout << "检验key成功\n";
        for (auto s : req.list()) {
            const int area = m_element_->GetPropertyInt(s.name(), excel::Server::Area());
            if (area == s.area()) { // 同一区服的就同步转发表
                int id = s.id();
                ServerInfo d;
                d.fd_ = sock;
                *d.info_ = s;
                servers_[id] = d;
                m_log_->LogInfo(Guid(0, s.id()), s.name(), " Registered");
            }
            else {
                m_log_->LogError(Guid(0, s.id()), s.name(), " Not Registered");
            }
        }
    }


    // 注册响应
    virtual void OnAckRegister(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
        Guid nPlayerID;
        rpc::AckRegisterServer ack;
        if (!m_net_->ReceivePB(msg_id, msg, len, ack, nPlayerID)) {
            return;
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

    // 连接socket事件
    void OnClientSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
        if (eEvent & SQUICK_NET_EVENT_EOF) {
        } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
            m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
            Register(pNet);
        }
    }

    // 向连接的服务注册自己
    void Register(INet *pNet) {
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

            for (auto &idx : idx_list) {
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
            auto s = req.add_list();
            *s = *sv.second.info_.get();
        }
        
        if (ts) {
            m_net_client_->SendToServerByPB(ts->id, rpc::ServerRPC::REQ_REGISTER, req);
            dout << pm_->GetAppName() <<  " 请求连接 " << ts->name << "\n";
            //m_log_->LogInfo(Guid(0, pm_->GetAppID()), s->name(), "Register");
        }
    }

    struct PlayerProxyInfo {
        int proxy_id;
        int proxy_sock;
    };

    // 玩家表
    map<Guid, PlayerProxyInfo> players_;

    // 服务表
    map<int, ServerInfo> servers_;

    public:
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