
#include "server_module.h"
#include "plugin.h"
#include <third_party/nlohmann/json.hpp>
MasterNet_ServerModule::~MasterNet_ServerModule() {}

bool MasterNet_ServerModule::Start() {
    this->pm_->SetAppType(ServerType::ST_MASTER);

    m_net_ = pm_->FindModule<INetModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();

    return true;
}

bool MasterNet_ServerModule::Destory() { return true; }

void MasterNet_ServerModule::OnWorldRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        std::shared_ptr<ServerData> pServerData = mWorldMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mWorldMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->nFD = sock;
        *(pServerData->pData) = xData;
        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "WorldRegistered");
    }

    SyncWorldToLoginAndWorld();
}

void MasterNet_ServerModule::OnWorldUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        mWorldMap.RemoveElement(xData.server_id());

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "WorldUnRegistered");
    }

    SyncWorldToLoginAndWorld();
}

void MasterNet_ServerModule::OnRefreshWorldInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        std::shared_ptr<ServerData> pServerData = mWorldMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mWorldMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->nFD = sock;
        *(pServerData->pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "RefreshWorldInfo");
    }

    SyncWorldToLoginAndWorld();
}

void MasterNet_ServerModule::OnLoginRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        std::shared_ptr<ServerData> pServerData = mLoginMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mLoginMap.AddElement(xData.server_id(), pServerData);
        }
        pServerData->nFD = sock;
        *(pServerData->pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "LoginRegistered");
    }
    SyncProxyToLogin();
    SyncWorldToLoginAndWorld();
}

void MasterNet_ServerModule::OnLoginUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);

        mLoginMap.RemoveElement(xData.server_id());

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "LoginUnRegistered");
    }
}

void MasterNet_ServerModule::OnRefreshLoginInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        std::shared_ptr<ServerData> pServerData = mLoginMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mLoginMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->nFD = sock;
        *(pServerData->pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "RefreshLoginInfo");
    }
}

bool MasterNet_ServerModule::Update() {
    static time_t last = 0;
    time_t now = SquickGetTimeS();
    if ( now - last > 10 ) {
        last = now;
        SyncProxyToLogin();
        SyncWorldToLoginAndWorld();
    }
    return true;
}

bool MasterNet_ServerModule::AfterStart() {
    m_net_->AddReceiveCallBack(SquickStruct::ServerRPC::STS_HEART_BEAT, this, &MasterNet_ServerModule::OnHeartBeat);
    m_net_->AddReceiveCallBack(SquickStruct::ServerRPC::WORLD_TO_MASTER_REGISTERED, this, &MasterNet_ServerModule::OnWorldRegisteredProcess);
    m_net_->AddReceiveCallBack(SquickStruct::ServerRPC::WORLD_TO_MASTER_UNREGISTERED, this, &MasterNet_ServerModule::OnWorldUnRegisteredProcess);
    m_net_->AddReceiveCallBack(SquickStruct::ServerRPC::WORLD_TO_MASTER_REFRESH, this, &MasterNet_ServerModule::OnRefreshWorldInfoProcess);
    m_net_->AddReceiveCallBack(SquickStruct::ServerRPC::LOGIN_TO_MASTER_REGISTERED, this, &MasterNet_ServerModule::OnLoginRegisteredProcess);
    m_net_->AddReceiveCallBack(SquickStruct::ServerRPC::LOGIN_TO_MASTER_UNREGISTERED, this, &MasterNet_ServerModule::OnLoginUnRegisteredProcess);
    m_net_->AddReceiveCallBack(SquickStruct::ServerRPC::LOGIN_TO_MASTER_REFRESH, this, &MasterNet_ServerModule::OnRefreshLoginInfoProcess);
    m_net_->AddReceiveCallBack(SquickStruct::ServerRPC::STS_SERVER_REPORT, this, &MasterNet_ServerModule::OnServerReport);

    m_net_->AddReceiveCallBack(this, &MasterNet_ServerModule::InvalidMessage);

    m_net_->AddEventCallBack(this, &MasterNet_ServerModule::OnSocketEvent);
    m_net_->ExpandBufferSize();

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::ST_MASTER && pm_->GetAppID() == serverID) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_element_->GetPropertyInt32(strId, excel::Server::CpuCount());
                const std::string &name = m_element_->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_element_->GetPropertyString(strId, excel::Server::IP());

                int nRet = m_net_->Startialization(maxConnect, nPort, nCpus);
                if (nRet < 0) {
                    std::ostringstream strLog;
                    strLog << "Cannot init server net, Port = " << nPort;
                    m_log_->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                    SQUICK_ASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
                    exit(0);
                }
                SquickStruct::ServerInfoReport reqMsg;

                reqMsg.set_server_id(serverID);
                reqMsg.set_server_name(strId);
                reqMsg.set_server_cur_count(0);
                reqMsg.set_server_ip(ip);
                reqMsg.set_server_port(nPort);
                reqMsg.set_server_max_online(maxConnect);
                reqMsg.set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
                reqMsg.set_server_type(serverType);

                auto pServerData = std::shared_ptr<ServerData>(new ServerData());
                *(pServerData->pData) = reqMsg;
                mMasterMap.AddElement(serverID, pServerData);
            }
        }
    }

    return true;
}

void MasterNet_ServerModule::OnSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    // std::cout << "OnSocketEvent::thread id=" << GetCurrentThreadId() << std::endl;

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

void MasterNet_ServerModule::OnClientDisconnect(const socket_t sock) {

    std::shared_ptr<ServerData> pServerData = mWorldMap.First();
    while (pServerData) {
        if (sock == pServerData->nFD) {
            pServerData->pData->set_server_state(SquickStruct::ServerState::SERVER_CRASH);
            pServerData->nFD = 0;

            SyncWorldToLoginAndWorld();
            return;
        }

        pServerData = mWorldMap.Next();
    }

    //////////////////////////////////////////////////////////////////////////

    int serverID = 0;
    pServerData = mLoginMap.First();
    while (pServerData) {
        if (sock == pServerData->nFD) {
            serverID = pServerData->pData->server_id();
            break;
        }

        pServerData = mLoginMap.Next();
    }

    mLoginMap.RemoveElement(serverID);
}

void MasterNet_ServerModule::OnClientConnected(const socket_t sock) {}

void MasterNet_ServerModule::SyncWorldToLoginAndWorld() {
    SquickStruct::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mWorldMap.First();
    while (pServerData) {
        SquickStruct::ServerInfoReport *pData = xData.add_server_list();
        *pData = *(pServerData->pData);
        pServerData = mWorldMap.Next();
    }

    // loginserver
    pServerData = mLoginMap.First();
    while (pServerData) {
        m_net_->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xData, pServerData->nFD);
        pServerData = mLoginMap.Next();
    }

    // world server
    pServerData = mWorldMap.First();
    while (pServerData) {
        SquickStruct::ServerInfoReportList xWorldData;
        const int nCurArea = m_element_->GetPropertyInt(pServerData->pData->server_name(), excel::Server::Area());

        for (int i = 0; i < xData.server_list_size(); ++i) {
            const SquickStruct::ServerInfoReport &xServerInfo = xData.server_list(i);
            // it must be the same area
            const int nAreaID = m_element_->GetPropertyInt(xServerInfo.server_name(), excel::Server::Area());
            if (nAreaID == nCurArea) {
                SquickStruct::ServerInfoReport *pData = xWorldData.add_server_list();
                *pData = *(pServerData->pData);
            }
        }
        m_net_->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xWorldData, pServerData->nFD);
        pServerData = mWorldMap.Next();
    }
}

void MasterNet_ServerModule::SyncProxyToLogin() {
    SquickStruct::ServerInfoReportList xData;
    std::shared_ptr<ServerData> pServerData = mProxyMap.First();
    while (pServerData) {
        SquickStruct::ServerInfoReport* pData = xData.add_server_list();
        *pData = *(pServerData->pData);
        pServerData = mProxyMap.Next();
    }

    // loginserver
    pServerData = mLoginMap.First();
    while (pServerData) {
        m_net_->SendMsgPB(SquickStruct::ServerRPC::STS_NET_INFO, xData, pServerData->nFD);
        pServerData = mLoginMap.Next();
    }
}

void MasterNet_ServerModule::LogGameServer() {
    if (mnLastLogTime + 60 > GetPluginManager()->GetNowTime()) {
        return;
    }

    mnLastLogTime = GetPluginManager()->GetNowTime();

    //////////////////////////////////////////////////////////////////////////

    m_log_->LogInfo(Guid(), "Begin Log WorldServer Info", "");

    std::shared_ptr<ServerData> pGameData = mWorldMap.First();
    while (pGameData) {
        std::ostringstream stream;
        stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id()
               << " State: " << SquickStruct::ServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip()
               << " FD: " << pGameData->nFD;
        m_log_->LogInfo(Guid(), stream);

        pGameData = mWorldMap.Next();
    }

    m_log_->LogInfo(Guid(), "End Log WorldServer Info", "");

    m_log_->LogInfo(Guid(), "Begin Log LoginServer Info", "");

    //////////////////////////////////////////////////////////////////////////
    pGameData = mLoginMap.First();
    while (pGameData) {
        std::ostringstream stream;
        stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id()
               << " State: " << SquickStruct::ServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip()
               << " FD: " << pGameData->nFD;
        m_log_->LogInfo(Guid(), stream);

        pGameData = mLoginMap.Next();
    }

    m_log_->LogInfo(Guid(), "End Log LoginServer Info", "");
}

void MasterNet_ServerModule::OnHeartBeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

void MasterNet_ServerModule::InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    printf("Net || umsg_id=%d\n", msg_id);
}

void MasterNet_ServerModule::OnServerReport(const socket_t nFd, const int msg_id, const char *buffer, const uint32_t len) {
    Guid xGUID;
    SquickStruct::ServerInfoReport msg;
    if (!m_net_->ReceivePB(msg_id, buffer, len, msg, xGUID)) {
        return;
    }

    std::shared_ptr<ServerData> pServerData;
    switch (msg.server_type()) {
    case ServerType::ST_LOGIN: {
        pServerData = mLoginMap.GetElement(msg.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mLoginMap.AddElement(msg.server_id(), pServerData);
        }
    } break;
    case ServerType::ST_WORLD: {
        pServerData = mWorldMap.GetElement(msg.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mWorldMap.AddElement(msg.server_id(), pServerData);
        }
    } break;
    case ServerType::ST_PROXY: {
        pServerData = mProxyMap.GetElement(msg.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mProxyMap.AddElement(msg.server_id(), pServerData);
        }
        
    } break;
    case ServerType::ST_GAME: {
        pServerData = mGameMap.GetElement(msg.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mGameMap.AddElement(msg.server_id(), pServerData);
        }
    } break;
    case ServerType::ST_GAMEPLAY_MANAGER: {
        pServerData = mGameplayManagerMap.GetElement(msg.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mGameplayManagerMap.AddElement(msg.server_id(), pServerData);
        }
    } break;

    default: {
        // m_log_->LogError("UNKNOW SERVER TYPE", msg.server_type());
    } break;
    }

    // udate status
    if (pServerData) {
        pServerData->nFD = nFd;
        *(pServerData->pData) = msg;
    }
}

// 获取服务状态
std::string MasterNet_ServerModule::GetServersStatus() {
    using json = nlohmann::json;
    json statusRoot;

    statusRoot["code"] = 0;
    statusRoot["msg"] = "";
    statusRoot["time"] = pm_->GetNowTime();

    std::shared_ptr<ServerData> pServerData = mMasterMap.First();
    int i = 0;
    while (pServerData) {
        json s;
        s["id"] = pServerData->pData->server_id();
        s["name"] = pServerData->pData->server_name().c_str();
        s["ip"] = pServerData->pData->server_ip().c_str();
        s["port"] = pServerData->pData->server_port();
        s["online"] = pServerData->pData->server_cur_count();
        s["status"] = (int)pServerData->pData->server_state();
        statusRoot["master" + std::to_string(i)] = s;
        i++;
        pServerData = mMasterMap.Next();
    }

    pServerData = mLoginMap.First();
    while (pServerData) {
        json s;
        s["id"] = pServerData->pData->server_id();
        s["name"] = pServerData->pData->server_name().c_str();
        s["ip"] = pServerData->pData->server_ip().c_str();
        s["port"] = pServerData->pData->server_port();
        s["online"] = pServerData->pData->server_cur_count();
        s["status"] = (int)pServerData->pData->server_state();
        statusRoot["login" + std::to_string(i)] = s;
        i++;
        pServerData = mLoginMap.Next();
    }

    pServerData = mWorldMap.First();
    while (pServerData.get()) {
        json s;
        s["id"] = pServerData->pData->server_id();
        s["name"] = pServerData->pData->server_name().c_str();
        s["ip"] = pServerData->pData->server_ip().c_str();
        s["port"] = pServerData->pData->server_port();
        s["online"] = pServerData->pData->server_cur_count();
        s["status"] = (int)pServerData->pData->server_state();
        statusRoot["world" + std::to_string(i)] = s;
        i++;
        pServerData = mWorldMap.Next();
    }

    pServerData = mProxyMap.First();
    while (pServerData.get()) {
        json s;
        s["id"] = pServerData->pData->server_id();
        s["name"] = pServerData->pData->server_name().c_str();
        s["ip"] = pServerData->pData->server_ip().c_str();
        s["port"] = pServerData->pData->server_port();
        s["online"] = pServerData->pData->server_cur_count();
        s["status"] = (int)pServerData->pData->server_state();
        statusRoot["proxy" + std::to_string(i)] = s;
        i++;
        pServerData = mProxyMap.Next();
    }

    pServerData = mGameMap.First();
    while (pServerData.get()) {
        json s;
        s["id"] = pServerData->pData->server_id();
        s["name"] = pServerData->pData->server_name().c_str();
        s["ip"] = pServerData->pData->server_ip().c_str();
        s["port"] = pServerData->pData->server_port();
        s["online"] = pServerData->pData->server_cur_count();
        s["status"] = (int)pServerData->pData->server_state();
        statusRoot["game" + std::to_string(i)] = s;
        i++;
        pServerData = mGameMap.Next();
    }

    pServerData = mGameplayManagerMap.First();
    while (pServerData.get()) {
        json s;
        s["id"] = pServerData->pData->server_id();
        s["name"] = pServerData->pData->server_name().c_str();
        s["ip"] = pServerData->pData->server_ip().c_str();
        s["port"] = pServerData->pData->server_port();
        s["online"] = pServerData->pData->server_cur_count();
        s["status"] = (int)pServerData->pData->server_state();
        statusRoot["gameplay_manager" + std::to_string(i)] = s;
        i++;
        pServerData = mGameplayManagerMap.Next();
    }

    return statusRoot.dump();
}
