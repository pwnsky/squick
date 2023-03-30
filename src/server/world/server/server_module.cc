
#include "server_module.h"
#include "plugin.h"
#include <squick/struct/struct.h>

bool WorldNet_ServerModule::Start() {
    this->pm_->SetAppType(ServerType::ST_WORLD);

    m_net_ = pm_->FindModule<INetModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_thread_pool_ = pm_->FindModule<IThreadPoolModule>();

    return true;
}

bool WorldNet_ServerModule::AfterStart() {
    m_net_->AddReceiveCallBack(rpc::PROXY_TO_WORLD_REGISTERED, this, &WorldNet_ServerModule::OnProxyServerRegisteredProcess);
    m_net_->AddReceiveCallBack(rpc::PROXY_TO_WORLD_UNREGISTERED, this, &WorldNet_ServerModule::OnProxyServerUnRegisteredProcess);
    m_net_->AddReceiveCallBack(rpc::PROXY_TO_WORLD_REFRESH, this, &WorldNet_ServerModule::OnRefreshProxyServerInfoProcess);

    m_net_->AddReceiveCallBack(rpc::GAME_TO_WORLD_REGISTERED, this, &WorldNet_ServerModule::OnGameServerRegisteredProcess);
    m_net_->AddReceiveCallBack(rpc::GAME_TO_WORLD_UNREGISTERED, this, &WorldNet_ServerModule::OnGameServerUnRegisteredProcess);
    m_net_->AddReceiveCallBack(rpc::GAME_TO_WORLD_REFRESH, this, &WorldNet_ServerModule::OnRefreshGameServerInfoProcess);

    m_net_->AddReceiveCallBack(rpc::DB_TO_WORLD_REGISTERED, this, &WorldNet_ServerModule::OnDBServerRegisteredProcess);
    m_net_->AddReceiveCallBack(rpc::DB_TO_WORLD_UNREGISTERED, this, &WorldNet_ServerModule::OnDBServerUnRegisteredProcess);
    m_net_->AddReceiveCallBack(rpc::DB_TO_WORLD_REFRESH, this, &WorldNet_ServerModule::OnRefreshDBServerInfoProcess);

    m_net_->AddReceiveCallBack(rpc::GAMEPLAY_MANAGER_TO_WORLD_REGISTERED, this,
                                     &WorldNet_ServerModule::OnGameplayManagerServerRegisteredProcess);
    m_net_->AddReceiveCallBack(rpc::GAMEPLAY_MANAGER_TO_WORLD_UNREGISTERED, this,
                                     &WorldNet_ServerModule::OnGameplayManagerServerUnRegisteredProcess);
    m_net_->AddReceiveCallBack(rpc::GAMEPLAY_MANAGER_TO_WORLD_REFRESH, this, &WorldNet_ServerModule::OnRefreshGameplayManagerServerInfoProcess);

    m_net_->AddReceiveCallBack(rpc::STS_SERVER_REPORT, this, &WorldNet_ServerModule::OnTransmitServerReport);

    m_net_->AddEventCallBack(this, &WorldNet_ServerModule::OnSocketEvent);
    m_net_->ExpandBufferSize();

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::ST_WORLD && pm_->GetAppID() == serverID) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_element_->GetPropertyInt32(strId, excel::Server::CpuCount());

                mAreaID = m_element_->GetPropertyInt32(strId, excel::Server::Area());
                // const std::string& name = m_element_->GetPropertyString(strId, SquickProtocol::Server::ID());
                // const std::string& ip = m_element_->GetPropertyString(strId, SquickProtocol::Server::IP());

                int nRet = m_net_->Startialization(maxConnect, nPort, nCpus);
                if (nRet < 0) {
                    std::ostringstream strLog;
                    strLog << "Cannot init server net, Port = " << nPort;
                    m_log_->LogError(NULL_OBJECT, strLog, __FUNCTION__, __LINE__);
                    SQUICK_ASSERT(nRet, "Cannot init server net", __FILE__, __FUNCTION__);
                    exit(0);
                }
            }
        }
    }

    return true;
}

void WorldNet_ServerModule::OnServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
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

            return;
        }

        const int nCurArea = m_element_->GetPropertyInt32(*itr, excel::Server::Area());

        Guid nPlayerID;
        rpc::ServerInfoReportList xMsg;
        if (!INetModule::ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
            return;
        }

        mWorldMap.ClearAll();

        for (int i = 0; i < xMsg.server_list_size(); ++i) {
            const rpc::ServerInfoReport &xData = xMsg.server_list(i);
            const int nAreaID = m_element_->GetPropertyInt(xData.server_name(), excel::Server::Area());
            if (xData.server_type() == ServerType::ST_WORLD && nCurArea == nAreaID) {
                std::shared_ptr<ServerData> pServerData = mWorldMap.GetElement(xData.server_id());
                if (!pServerData) {
                    pServerData = std::shared_ptr<ServerData>(new ServerData());
                    mWorldMap.AddElement(xData.server_id(), pServerData);
                }

                pServerData->nFD = sock;
                *(pServerData->pData) = xData;

                m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
            }
        }
    }

    // sync to proxy
    SynWorldToProxy();

    // sync to game
    SynWorldToGame();

    // 同步 World 到 Gameplay Manager
    SynWorldToGameplayManager();
}

bool WorldNet_ServerModule::Destory() { return true; }

bool WorldNet_ServerModule::Update() {

    if (mnLastCheckTime + 10 > GetPluginManager()->GetNowTime()) {
        return true;
    }
    // 定时同步服务器表

    SynDBToGame();
    SynGameToProxy();
    SynWorldToProxy();
    SynWorldToGame();
    SynWorldToDB();

    SynGameToGameplayManager();
    SynWorldToGameplayManager();

    // LogGameServer();

    return true;
}

void WorldNet_ServerModule::OnGameServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
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

            return;
        }

        const int nCurArea = m_element_->GetPropertyInt32(*itr, excel::Server::Area());

        Guid nPlayerID;
        rpc::ServerInfoReportList xMsg;
        if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
            return;
        }

        for (int i = 0; i < xMsg.server_list_size(); ++i) {
            const rpc::ServerInfoReport &xData = xMsg.server_list(i);
            const int nAreaID = m_element_->GetPropertyInt(xData.server_name(), excel::Server::Area());
            if (nAreaID == nCurArea) {
                std::shared_ptr<ServerData> pServerData = mGameMap.GetElement(xData.server_id());
                if (!pServerData) {

                    pServerData = std::shared_ptr<ServerData>(new ServerData());
                    mGameMap.AddElement(xData.server_id(), pServerData);
                }

                pServerData->nFD = sock;
                *(pServerData->pData) = xData;

                m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
            } else {
                m_log_->LogError(Guid(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
            }
        }
    }

    SynGameToProxy();
}

void WorldNet_ServerModule::OnGameServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    rpc::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const rpc::ServerInfoReport &xData = xMsg.server_list(i);
        mGameMap.RemoveElement(xData.server_id());

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
    }
}

void WorldNet_ServerModule::OnRefreshGameServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    rpc::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const rpc::ServerInfoReport &xData = xMsg.server_list(i);

        std::shared_ptr<ServerData> pServerData = mGameMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mGameMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->nFD = sock;
        *(pServerData->pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "GameServerRegistered");
    }

    SynGameToProxy();
}

void WorldNet_ServerModule::OnProxyServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
   
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

            return;
        }

        const int nCurArea = m_element_->GetPropertyInt32(*itr, excel::Server::Area());

        Guid nPlayerID;
        rpc::ServerInfoReportList xMsg;
        if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
            return;
        }

        for (int i = 0; i < xMsg.server_list_size(); ++i) {
            const rpc::ServerInfoReport &xData = xMsg.server_list(i);
            const int nAreaID = m_element_->GetPropertyInt(xData.server_name(), excel::Server::Area());
            if (nAreaID == nCurArea) // 同一区服的就同步转发表
            {
                std::shared_ptr<ServerData> server = mProxyMap.GetElement(xData.server_id());
                if (!server) {
                    server = std::shared_ptr<ServerData>(new ServerData());

                    mProxyMap.AddElement(xData.server_id(), server);
                }
                
                server->nFD = sock;
                *(server->pData) = xData;

                dout << "代理服务器请求在World上注册: " << server->pData->server_id() << " ServerName: " << server->pData->server_name() << std::endl;
                m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");
                SynGameToProxy(sock);
            } else {
                m_log_->LogError(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");
            }
        }
    }
}

void WorldNet_ServerModule::OnProxyServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    rpc::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const rpc::ServerInfoReport &xData = xMsg.server_list(i);

        mGameMap.RemoveElement(xData.server_id());

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy UnRegistered");
    }
}

void WorldNet_ServerModule::OnRefreshProxyServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    rpc::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const rpc::ServerInfoReport &xData = xMsg.server_list(i);

        std::shared_ptr<ServerData> pServerData = mProxyMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mProxyMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->nFD = sock;
        *(pServerData->pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");

        SynGameToProxy(sock);
    }
}

void WorldNet_ServerModule::OnDBServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
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

            return;
        }

        const int nCurArea = m_element_->GetPropertyInt32(*itr, excel::Server::Area());

        Guid nPlayerID;
        rpc::ServerInfoReportList xMsg;
        if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
            return;
        }

        for (int i = 0; i < xMsg.server_list_size(); ++i) {
            const rpc::ServerInfoReport &xData = xMsg.server_list(i);
            const int nAreaID = m_element_->GetPropertyInt(xData.server_name(), excel::Server::Area());
            if (nAreaID == nCurArea) {

                std::shared_ptr<ServerData> pServerData = mDBMap.GetElement(xData.server_id());
                if (!pServerData) {
                    pServerData = std::shared_ptr<ServerData>(new ServerData());
                    mDBMap.AddElement(xData.server_id(), pServerData);
                }

                pServerData->nFD = sock;
                *(pServerData->pData) = xData;

                m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "DBServerRegistered");
            } else {
                m_log_->LogError(Guid(0, xData.server_id()), xData.server_name(), "DBServerRegistered");
            }
        }

        SynDBToGame();
    }
}

void WorldNet_ServerModule::OnDBServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    rpc::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const rpc::ServerInfoReport &xData = xMsg.server_list(i);

        mDBMap.RemoveElement(xData.server_id());

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy UnRegistered");
    }
}

void WorldNet_ServerModule::OnRefreshDBServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    rpc::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const rpc::ServerInfoReport &xData = xMsg.server_list(i);

        std::shared_ptr<ServerData> pServerData = mDBMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mDBMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->nFD = sock;
        *(pServerData->pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");

        SynDBToGame(sock);
        // SynDBToPvp()
    }
}

// PVP  Server

void WorldNet_ServerModule::OnGameplayManagerServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
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

            return;
        }

        const int nCurArea = m_element_->GetPropertyInt32(*itr, excel::Server::Area());

        Guid nPlayerID;
        rpc::ServerInfoReportList xMsg;
        if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
            return;
        }

        for (int i = 0; i < xMsg.server_list_size(); ++i) {
            const rpc::ServerInfoReport &xData = xMsg.server_list(i);
            const int nAreaID = m_element_->GetPropertyInt(xData.server_name(), excel::Server::Area());
            if (nAreaID == nCurArea) // 同一区服的就同步转发表
            {
                std::shared_ptr<ServerData> pServerData = mGameplayManagerMap.GetElement(xData.server_id());
                if (!pServerData) {
                    pServerData = std::shared_ptr<ServerData>(new ServerData());
                    mGameplayManagerMap.AddElement(xData.server_id(), pServerData);
                }

                pServerData->nFD = sock;
                *(pServerData->pData) = xData;

                m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");

                SynGameToProxy(sock);
            } else {
                m_log_->LogError(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");
            }
        }
    }
}

void WorldNet_ServerModule::OnGameplayManagerServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    rpc::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const rpc::ServerInfoReport &xData = xMsg.server_list(i);

        mGameMap.RemoveElement(xData.server_id());

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy UnRegistered");
    }
}

void WorldNet_ServerModule::OnRefreshGameplayManagerServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    rpc::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const rpc::ServerInfoReport &xData = xMsg.server_list(i);

        std::shared_ptr<ServerData> pServerData = mGameplayManagerMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ServerData>(new ServerData());
            mGameplayManagerMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->nFD = sock;
        *(pServerData->pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");

        SynGameToProxy(sock);
    }
}
// -------------------

void WorldNet_ServerModule::OnSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
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

// 同步转发表
void WorldNet_ServerModule::SynGameToProxy() {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mProxyMap.First();
    while (pServerData) {
        SynGameToProxy(pServerData->nFD);
        pServerData = mProxyMap.Next();
    }
}

void WorldNet_ServerModule::SynGameToProxy(const socket_t nFD) {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mGameMap.First();
    while (pServerData) {
        rpc::ServerInfoReport *pData = xData.add_server_list();
        *pData = *(pServerData->pData);
        pServerData = mGameMap.Next();
    }

    m_net_->SendMsgPB(rpc::ServerRPC::STS_NET_INFO, xData, nFD);
}

void WorldNet_ServerModule::SynWorldToProxy() {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mProxyMap.First();
    while (pServerData) {
        SynWorldToProxy(pServerData->nFD);

        pServerData = mProxyMap.Next();
    }
}

void WorldNet_ServerModule::SynWorldToProxy(const socket_t nFD) {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mWorldMap.First();
    while (pServerData) {
        rpc::ServerInfoReport *pData = xData.add_server_list();
        *pData = *(pServerData->pData);
        pServerData = mWorldMap.Next();
    }

    m_net_->SendMsgPB(rpc::ServerRPC::STS_NET_INFO, xData, nFD);
}

// Start: PVP Manager <-> Game < - > World 服务器之间的转发表同步

void WorldNet_ServerModule::SynGameToGameplayManager() {
    rpc::ServerInfoReportList xData;
    // 依次同步到 Pvp Manager 服务器上
    std::shared_ptr<ServerData> pServerData = mGameplayManagerMap.First();
    while (pServerData) {
        SynGameToGameplayManager(pServerData->nFD);
        pServerData = mGameplayManagerMap.Next();
    }
}

void WorldNet_ServerModule::SynGameToGameplayManager(const socket_t nFD) {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mGameMap.First();
    while (pServerData) {
        rpc::ServerInfoReport *pData = xData.add_server_list();
        *pData = *(pServerData->pData);
        pServerData = mGameMap.Next();
    }

    m_net_->SendMsgPB(rpc::ServerRPC::STS_NET_INFO, xData, nFD);
}

void WorldNet_ServerModule::SynWorldToGameplayManager() {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mGameplayManagerMap.First();
    while (pServerData) {
        SynWorldToGameplayManager(pServerData->nFD);

        pServerData = mGameplayManagerMap.Next();
    }
}

void WorldNet_ServerModule::SynWorldToGameplayManager(const socket_t nFD) {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mWorldMap.First();
    while (pServerData) {
        rpc::ServerInfoReport *pData = xData.add_server_list();
        *pData = *(pServerData->pData);

        pServerData = mWorldMap.Next();
    }

    m_net_->SendMsgPB(rpc::ServerRPC::STS_NET_INFO, xData, nFD);
}

// End: PVP Manager <-> Game < - > World

void WorldNet_ServerModule::SynWorldToGame() {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mGameMap.First();
    while (pServerData) {
        if (pServerData->pData->server_state() != rpc::ServerState::SERVER_MAINTEN &&
            pServerData->pData->server_state() != rpc::ServerState::SERVER_CRASH) {
            SynWorldToGame(pServerData->nFD);
        }
        pServerData = mGameMap.Next();
    }
}

void WorldNet_ServerModule::SynWorldToGame(const socket_t nFD) {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mWorldMap.First();
    while (pServerData) {
        rpc::ServerInfoReport *pData = xData.add_server_list();
        *pData = *(pServerData->pData);

        pServerData = mWorldMap.Next();
    }

    m_net_->SendMsgPB(rpc::ServerRPC::STS_NET_INFO, xData, nFD);
}

void WorldNet_ServerModule::SynWorldToDB() {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mDBMap.First();
    while (pServerData) {
        if (pServerData->pData->server_state() != rpc::ServerState::SERVER_MAINTEN &&
            pServerData->pData->server_state() != rpc::ServerState::SERVER_CRASH) {
            SynWorldToDB(pServerData->nFD);
        }

        pServerData = mDBMap.Next();
    }
}

void WorldNet_ServerModule::SynWorldToDB(const socket_t nFD) {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mWorldMap.First();
    while (pServerData) {

        rpc::ServerInfoReport *pData = xData.add_server_list();
        *pData = *(pServerData->pData);

        pServerData = mWorldMap.Next();
    }

    m_net_->SendMsgPB(rpc::ServerRPC::STS_NET_INFO, xData, nFD);
}

void WorldNet_ServerModule::SynDBToGame() {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mGameMap.First();
    while (pServerData) {
        if (pServerData->pData->server_state() != rpc::ServerState::SERVER_MAINTEN &&
            pServerData->pData->server_state() != rpc::ServerState::SERVER_CRASH) {
            SynDBToGame(pServerData->nFD);
        }

        pServerData = mGameMap.Next();
    }
}

void WorldNet_ServerModule::SynDBToGame(const socket_t nFD) {
    rpc::ServerInfoReportList xData;

    std::shared_ptr<ServerData> pServerData = mDBMap.First();
    while (pServerData) {
        rpc::ServerInfoReport *pData = xData.add_server_list();
        *pData = *(pServerData->pData);
        pServerData = mDBMap.Next();
    }

    m_net_->SendMsgPB(rpc::ServerRPC::STS_NET_INFO, xData, nFD);
}

void WorldNet_ServerModule::OnClientDisconnect(const socket_t sock) {
    std::shared_ptr<ServerData> pServerData = mGameMap.First();
    while (pServerData) {
        if (sock == pServerData->nFD) {
            pServerData->pData->set_server_state(rpc::ServerState::SERVER_CRASH);
            pServerData->nFD = 0;

            ServerReport(pServerData->pData->server_id(), rpc::ServerState::SERVER_CRASH);
            SynGameToProxy();
            break;
        }

        pServerData = mGameMap.Next();
    }

    //////////////////////////////////////////////////////////////////////////

    pServerData = mProxyMap.First();
    while (pServerData) {
        if (sock == pServerData->nFD) {
            pServerData->pData->set_server_state(rpc::ServerState::SERVER_CRASH);
            pServerData->nFD = 0;

            int serverID = pServerData->pData->server_id();
            mProxyMap.RemoveElement(serverID);

            ServerReport(pServerData->pData->server_id(), rpc::ServerState::SERVER_CRASH);
            SynGameToProxy();
            break;
        }

        pServerData = mProxyMap.Next();
    }

    //////////////////////////////////////////////////////////////////////////

    pServerData = mDBMap.First();
    while (pServerData) {
        if (sock == pServerData->nFD) {
            pServerData->pData->set_server_state(rpc::ServerState::SERVER_CRASH);
            pServerData->nFD = 0;

            int serverID = pServerData->pData->server_id();
            mDBMap.RemoveElement(serverID);

            ServerReport(pServerData->pData->server_id(), rpc::ServerState::SERVER_CRASH);
            SynDBToGame();
            break;
        }

        pServerData = mDBMap.Next();
    }
}

void WorldNet_ServerModule::OnClientConnected(const socket_t sock) {}

void WorldNet_ServerModule::LogGameServer() {
    mnLastCheckTime = GetPluginManager()->GetNowTime();

    m_log_->LogInfo(Guid(), "--------------------Begin Log GameServer Info", "");

    std::shared_ptr<ServerData> pGameData = mGameMap.First();
    while (pGameData) {
        std::ostringstream stream;
        stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id()
               << " State: " << rpc::ServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip()
               << " FD: " << pGameData->nFD;

        m_log_->LogInfo(Guid(), stream);

        pGameData = mGameMap.Next();
    }

    m_log_->LogInfo(Guid(), "--------------------End Log GameServer Info", "");

    m_log_->LogInfo(Guid(), "--------------------Begin Log ProxyServer Info", "");

    pGameData = mProxyMap.First();
    while (pGameData) {
        std::ostringstream stream;
        stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id()
               << " State: " << rpc::ServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip()
               << " FD: " << pGameData->nFD;

        m_log_->LogInfo(Guid(), stream);

        pGameData = mProxyMap.Next();
    }

    m_log_->LogInfo(Guid(), "--------------------End Log ProxyServer Info", "");

    m_log_->LogInfo(Guid(), "--------------------Begin Log DBServer Info", "");

    pGameData = mDBMap.First();
    while (pGameData) {
        std::ostringstream stream;
        stream << "Type: " << pGameData->pData->server_type() << " ID: " << pGameData->pData->server_id()
               << " State: " << rpc::ServerState_Name(pGameData->pData->server_state()) << " IP: " << pGameData->pData->server_ip()
               << " FD: " << pGameData->nFD;

        m_log_->LogInfo(Guid(), stream);

        pGameData = mDBMap.Next();
    }

    m_log_->LogInfo(Guid(), "--------------------End Log DBServer Info", "");
}

void WorldNet_ServerModule::OnOnlineProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    /*
    std::cout << "ONline::::::\n";
  CLIENT_MSG_PROCESS_NO_OBJECT(msg_id, msg, len, rpc::RoleOnlineNotify);

  Guid selfId = INetModule::ProtobufToStruct(xMsg.self());

  std::shared_ptr<PlayerData> playerData = mPlayersData.GetElement(selfId);
  if (playerData)
  {
            playerData->name = xMsg.name();
            playerData->bp = xMsg.bp();

  }
    else
    {
            playerData = std::shared_ptr<PlayerData>(new PlayerData(selfId));

            playerData->name = xMsg.name();
            playerData->bp = xMsg.bp();

            mPlayersData.AddElement(selfId, playerData);
    }

    playerData->OnLine(xMsg.game(), xMsg.proxy());

    for (int i = 0; i < mPlayerOnLineCallBackFunc.size(); ++i)
    {
            auto callback = mPlayerOnLineCallBackFunc[i];
            callback->operator()(selfId);
    } */
}

void WorldNet_ServerModule::OnOfflineProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    /*
    std::cout << "Offline::::::\n";
  CLIENT_MSG_PROCESS_NO_OBJECT(msg_id, msg, len, rpc::RoleOfflineNotify);
  Guid self = INetModule::ProtobufToStruct(xMsg.self());


    std::shared_ptr<PlayerData> playerData = mPlayersData.GetElement(self);
    if (playerData)
    {
            for (int i = 0; i < mPlayerOffLineCallBackFunc.size(); ++i)
            {
                    auto callback = mPlayerOffLineCallBackFunc[i];
                    callback->operator()(self);
            }


            playerData->OffLine();
    } */
}

void WorldNet_ServerModule::OnTransmitServerReport(const socket_t nFd, const int msg_id, const char *buffer, const uint32_t len) {
    Guid xGUID;
    rpc::ServerInfoReport msg;
    if (!m_net_->ReceivePB(msg_id, buffer, len, msg, xGUID)) {
        return;
    }

    m_net_client_->SendToAllServerByPB(ServerType::ST_MASTER, rpc::STS_SERVER_REPORT, msg, Guid());
}

bool WorldNet_ServerModule::SendMsgToGame(const int gameID, const int msg_id, const std::string &xData) {
    std::shared_ptr<ServerData> pData = mGameMap.GetElement(gameID);
    if (pData) {
        const socket_t nFD = pData->nFD;
        m_net_->SendMsg(msg_id, xData, nFD, Guid());

        return true;
    }

    return false;
}

bool WorldNet_ServerModule::SendMsgToGame(const int gameID, const int msg_id, const google::protobuf::Message &xData) {
    std::shared_ptr<ServerData> pData = mGameMap.GetElement(gameID);
    if (pData) {
        const socket_t nFD = pData->nFD;
        m_net_->SendMsgPB(msg_id, xData, nFD, Guid());

        return true;
    }

    return false;
}

bool WorldNet_ServerModule::SendMsgToGamePlayer(const Guid nPlayer, const int msg_id, const std::string &xData) {
    std::shared_ptr<PlayerData> playerData = mPlayersData.GetElement(nPlayer);
    if (playerData) {
        std::shared_ptr<ServerData> pData = mGameMap.GetElement(playerData->gameID);
        if (pData) {
            const socket_t nFD = pData->nFD;
            m_net_->SendMsg(msg_id, xData, nFD, nPlayer);

            return true;
        }
    }

    return false;
}

bool WorldNet_ServerModule::SendMsgToGamePlayer(const Guid nPlayer, const int msg_id, const google::protobuf::Message &xData) {
    std::shared_ptr<PlayerData> playerData = mPlayersData.GetElement(nPlayer);
    if (playerData) {
        std::shared_ptr<ServerData> pData = mGameMap.GetElement(playerData->gameID);
        if (pData) {
            const socket_t nFD = pData->nFD;
            m_net_->SendMsgPB(msg_id, xData, nFD, nPlayer);

            return true;
        }
    }

    return false;
}

bool WorldNet_ServerModule::SendMsgToGamePlayer(const DataList &argObjectVar, const int msg_id, google::protobuf::Message &xData) {
    for (int i = 0; i < argObjectVar.GetCount(); i++) {
        const Guid &nPlayer = argObjectVar.Object(i);
        SendMsgToGamePlayer(nPlayer, msg_id, xData);
    }

    return true;
}

std::shared_ptr<ServerData> WorldNet_ServerModule::GetSuitProxyToEnter() {
    int nConnectNum = 99999;
    std::shared_ptr<ServerData> pReturnServerData;

    std::shared_ptr<ServerData> pServerData = mProxyMap.First();
    while (pServerData) {
        if (pServerData->pData->server_cur_count() < nConnectNum) {
            nConnectNum = pServerData->pData->server_cur_count();
            pReturnServerData = pServerData;
        }

        pServerData = mProxyMap.Next();
    }

    return pReturnServerData;
}

std::shared_ptr<ServerData> WorldNet_ServerModule::GetSuitGameToEnter(const int arg) { return mGameMap.GetElementBySuit(arg); }

int WorldNet_ServerModule::GetPlayerGameID(const Guid self) {
    std::shared_ptr<PlayerData> playerData = mPlayersData.GetElement(self);
    if (playerData) {
        return playerData->gameID;
    }

    return 0;
}

const std::vector<Guid> &WorldNet_ServerModule::GetOnlinePlayers() {
    static std::vector<Guid> players;
    players.clear();

    std::shared_ptr<PlayerData> playerData = mPlayersData.First();
    while (playerData) {
        players.push_back(playerData->self);

        playerData = mPlayersData.Next();
    }

    return players;
}

std::shared_ptr<IWorldNet_ServerModule::PlayerData> WorldNet_ServerModule::GetPlayerData(const Guid &id) {
    // 如果没有，就从redis load

    return mPlayersData.GetElement(id);
}

void WorldNet_ServerModule::ServerReport(int reportServerId, rpc::ServerState serverStatus) {
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (reportServerId == serverID) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const std::string &name = m_element_->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_element_->GetPropertyString(strId, excel::Server::IP());

                rpc::ServerInfoReport reqMsg;

                reqMsg.set_server_id(serverID);
                reqMsg.set_server_name(strId);
                reqMsg.set_server_cur_count(0);
                reqMsg.set_server_ip(ip);
                reqMsg.set_server_port(nPort);
                reqMsg.set_server_max_online(maxConnect);
                reqMsg.set_server_state(serverStatus);
                reqMsg.set_server_type(serverType);

                m_net_client_->SendToAllServerByPB(ServerType::ST_MASTER, rpc::STS_SERVER_REPORT, reqMsg, Guid());
            }
        }
    }
}

bool WorldNet_ServerModule::AddOnLineReceiveCallBack(std::shared_ptr<std::function<void(const Guid)>> cb) {
    mPlayerOnLineCallBackFunc.push_back(cb);
    return true;
}

bool WorldNet_ServerModule::AddOffLineReceiveCallBack(std::shared_ptr<std::function<void(const Guid)>> cb) {
    mPlayerOffLineCallBackFunc.push_back(cb);
    return true;
}

bool WorldNet_ServerModule::IsPrimaryWorldServer() {
    auto serverData = mWorldMap.GetElementBySuitConsistent();
    if (serverData && serverData->pData->server_id() == GetPluginManager()->GetAppID()) {
        return true;
    }

    return false;
}

int WorldNet_ServerModule::GetWorldAreaID() { return mAreaID; }
