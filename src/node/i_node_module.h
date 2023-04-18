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
#include <squick/struct/struct.h>

class INodeModule : public IModule {

    //////////////////////////// PUBLIC ////////////////////////////
  public:
    virtual bool Start() final {
        m_net_ = pm_->FindModule<INetModule>();
        m_kernel_ = pm_->FindModule<IKernelModule>();
        m_log_ = pm_->FindModule<ILogModule>();
        m_element_ = pm_->FindModule<IElementModule>();
        m_class_ = pm_->FindModule<IClassModule>();
        m_net_client_ = pm_->FindModule<INetClientModule>();
        m_thread_pool_ = pm_->FindModule<IThreadPoolModule>();
        
        m_update_ = true;
        return true;
    }

    virtual bool Update() final {
        if (last_report_time_ + 15 > pm_->GetNowTime()) {
            return true;
        }
        ServerReport();
        return true;
    }

    bool Listen() {
        m_net_->AddReceiveCallBack(rpc::REQ_REGISTER_SERVER, this, &INodeModule::OnServerRegistered);
        m_net_->AddReceiveCallBack(rpc::REQ_UNREGISTER_SERVER, this, &INodeModule::OnServerUnRegistered);
        m_net_->AddReceiveCallBack(rpc::SERVER_REPORT, this, &INodeModule::OnServerReport);
        m_net_->AddEventCallBack(this, &INodeModule::OnServerSocketEvent);
        m_net_->ExpandBufferSize();

        std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
        if (xLogicClass) {
            const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
            for (int i = 0; i < strIdList.size(); ++i) {
                const std::string &strId = strIdList[i];

                const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
                if (pm_->GetAppID() == serverID) {
                    info_.type = (ServerType)m_element_->GetPropertyInt32(strId, excel::Server::Type());
                    info_.port = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                    info_.max_connect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                    info_.cpu_count = m_element_->GetPropertyInt32(strId, excel::Server::CpuCount());
                    info_.area = m_element_->GetPropertyInt32(strId, excel::Server::Area());
                    info_.name = m_element_->GetPropertyString(strId, excel::Server::ID());
                    info_.ip = m_element_->GetPropertyString(strId, excel::Server::IP());

                    pm_->SetAppType(info_.type);

                    int nRet = m_net_->Startialization(info_.max_connect, info_.port, info_.cpu_count);
                    if (nRet < 0) {
                        std::ostringstream strLog;
                        strLog << "Cannot init server net, Port = " << info_.port;
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
            s.nGameID = xData.id();
            s.ip = xData.ip();
            s.nPort = xData.port();
            s.name = xData.name();
            s.nWorkLoad = xData.cpu_count();
            s.eServerType = (ServerType)xData.type();
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

                    m_net_client_->SendToAllServerByPB(ServerType::ST_WORLD, rpc::SERVER_REPORT, s, Guid());
                }
            }
        }
    }

    // Add upper server
    bool AddServer(ServerType type) {
        m_net_client_->AddEventCallBack(type, this, &INodeClientModule::OnClientSocketEvent);
        m_net_client_->AddReceiveCallBack(type, rpc::SERVER_ADD, this, &INodeClientModule::OnDynamicServerAdd);
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

                const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
                const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
                const int nServerArea = m_element_->GetPropertyInt32(strId, excel::Server::Area());
                if (serverType == type && nCurArea == nServerArea) { // 同一区服注册
                    const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                    const std::string &name = m_element_->GetPropertyString(strId, excel::Server::ID());
                    const std::string &ip = m_element_->GetPropertyString(strId, excel::Server::IP());
                    ConnectData s;
                    s.nGameID = serverID;
                    s.eServerType = (ServerType)serverType;
                    s.ip = ip;
                    s.nPort = nPort;
                    s.name = strId;
                    m_net_client_->AddServer(s);
                    return true;
                }
            }
        }
        return false;
    }

    virtual void OnClientDisconnect(socket_t sock) = 0;
    virtual void OnClientConnected(socket_t sock) = 0;

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

    //////////////////////////// PRIVATE ////////////////////////////
  private:
    virtual void OnServerRefreshed(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        Guid nPlayerID;
        rpc::ServerList sl;
        if (!m_net_->ReceivePB(msg_id, msg, len, sl, nPlayerID)) {
            return;
        }
        for (int i = 0; i < sl.list_size(); ++i) {
            const rpc::Server &s = sl.list(i);
            int id = s.id();
            ServerData d;
            d.nFD = sock;
            *d.pData = s;
            servers_[id] = d;
            m_log_->LogInfo(Guid(0, s.id()), s.name(), " Refreshed");
        }
    }

    virtual void OnServerReport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

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

    virtual void OnServerRegistered(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
        Guid nPlayerID;
        rpc::ServerList sl;
        if (!m_net_->ReceivePB(msg_id, msg, len, sl, nPlayerID)) {
            return;
        }
        for (int i = 0; i < sl.list_size(); ++i) {
            const rpc::Server &s = sl.list(i);
            const int area = m_element_->GetPropertyInt(s.name(), excel::Server::Area());
            if (area == info_.area) { // 同一区服的就同步转发表
                int id = s.id();
                ServerData d;
                d.nFD = sock;
                *d.pData = s;
                servers_[id] = d;
                m_log_->LogInfo(Guid(0, s.id()), s.name(), " Registered");
            } else {
                m_log_->LogError(Guid(0, s.id()), s.name(), " Not Registered");
            }
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

    // 连接后注册
    void OnClientSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
        if (eEvent & SQUICK_NET_EVENT_EOF) {
        } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
            m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
            Register(pNet);
        }
    }

    // Register server
    void Register(INet *pNet) {
        std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
        if (xLogicClass) {
            const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
            for (int i = 0; i < strIdList.size(); ++i) {
                const std::string &strId = strIdList[i];

                const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
                if (pm_->GetAppID() == serverID) {
                    const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
                    const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                    const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                    const std::string &name = m_element_->GetPropertyString(strId, excel::Server::ID());
                    const std::string &ip = m_element_->GetPropertyString(strId, excel::Server::IP());

                    rpc::ServerList xMsg;
                    rpc::Server *s = xMsg.add_list();

                    s->set_id(serverID);
                    s->set_name(strId);
                    s->set_cpu_count(0);
                    s->set_ip(ip);
                    s->set_port(nPort);
                    s->set_max_online(maxConnect);
                    s->set_state(rpc::ServerState::ServerNormal);
                    s->set_type(serverType);

                    std::shared_ptr<ConnectData> pServerData = m_net_client_->GetServerNetInfo(pNet);
                    if (pServerData) {
                        int nTargetID = pServerData->nGameID;
                        m_net_client_->SendToServerByPB(nTargetID, rpc::ServerRPC::REQ_REGISTER_SERVER, xMsg);
                        m_log_->LogInfo(Guid(0, s->id()), s->name(), "Register");
                    }
                }
            }
        }
    }

    struct PlayerProxyInfo {
        int proxy_id;
        int proxy_sock;
    };

    // 玩家表
    map<Guid, PlayerProxyInfo> players_;

    // 服务表
    map<int, ServerData> servers_;
    struct ServerInfo {
        int id = 0;
        ServerType type = ServerType::ST_NONE;
        int area = 0;
        int port = 0;
        int max_connect = 0;
        int cpu_count = 0;
        string ip;
        string name;
    };
    ServerInfo info_;

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