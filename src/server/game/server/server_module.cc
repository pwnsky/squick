#include "server_module.h"
#include <forward_list>
#include <squick/plugin/kernel/i_event_module.h>
#include <squick/plugin/kernel/scene_module.h>

namespace game::server {
bool GameServerNet_ServerModule::Start() {
    this->pm_->SetAppType(ServerType::SQUICK_ST_GAME);

    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_event_ = pm_->FindModule<IEventModule>();
    m_scene_ = pm_->FindModule<ISceneModule>();
    m_schedule_ = pm_->FindModule<IScheduleModule>();

    m_net_ = pm_->FindModule<INetModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_pThreadPoolModule = pm_->FindModule<IThreadPoolModule>();

    return true;
}

bool GameServerNet_ServerModule::AfterStart() {

    m_net_->AddReceiveCallBack(SquickStruct::PROXY_TO_GAME_REFRESH, this, &GameServerNet_ServerModule::OnRefreshProxyServerInfoProcess);
    m_net_->AddReceiveCallBack(SquickStruct::PROXY_TO_GAME_REGISTERED, this, &GameServerNet_ServerModule::OnProxyServerRegisteredProcess);
    m_net_->AddReceiveCallBack(SquickStruct::PROXY_TO_GAME_UNREGISTERED, this, &GameServerNet_ServerModule::OnProxyServerUnRegisteredProcess);

    m_net_->AddReceiveCallBack(SquickStruct::GAMEPLAY_MANAGER_TO_GAME_REFRESH, this, &GameServerNet_ServerModule::OnRefreshPvpManagerServerInfoProcess);
    m_net_->AddReceiveCallBack(SquickStruct::GAMEPLAY_MANAGER_TO_GAME_REGISTERED, this, &GameServerNet_ServerModule::OnPvpManagerServerRegisteredProcess);
    m_net_->AddReceiveCallBack(SquickStruct::GAMEPLAY_MANAGER_TO_GAME_UNREGISTERED, this,
                                     &GameServerNet_ServerModule::OnPvpManagerServerUnRegisteredProcess);

    m_net_->AddEventCallBack(this, &GameServerNet_ServerModule::OnSocketPSEvent);

    /////////////////////////////////////////////////////////////////////////

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == ServerType::SQUICK_ST_GAME && pm_->GetAppID() == serverID) {
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_element_->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_element_->GetPropertyInt32(strId, excel::Server::CpuCount());
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

bool GameServerNet_ServerModule::Destory() { return true; }

bool GameServerNet_ServerModule::Update() { return true; }

void GameServerNet_ServerModule::OnSocketPSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
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

void GameServerNet_ServerModule::OnClientDisconnect(const socket_t sock) {
    int serverID = 0;
    std::shared_ptr<ProxyServerInfo> pServerData = mProxyMap.First();
    while (pServerData) {
        if (sock == pServerData->xServerData.nFD) {
            serverID = pServerData->xServerData.pData->server_id();
            break;
        }

        pServerData = mProxyMap.Next();
    }
    mProxyMap.RemoveElement(serverID);
}

void GameServerNet_ServerModule::OnClientConnected(const socket_t sock) {}

void GameServerNet_ServerModule::OnProxyServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!INetModule::ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        std::shared_ptr<ProxyServerInfo> pServerData = mProxyMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ProxyServerInfo>(new ProxyServerInfo());
            mProxyMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->xServerData.nFD = sock;
        *(pServerData->xServerData.pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");
    }

    return;
}

void GameServerNet_ServerModule::OnProxyServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        mProxyMap.RemoveElement(xData.server_id());

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy UnRegistered");
    }

    return;
}

void GameServerNet_ServerModule::OnRefreshProxyServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        std::shared_ptr<ProxyServerInfo> pServerData = mProxyMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ProxyServerInfo>(new ProxyServerInfo());
            mProxyMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->xServerData.nFD = sock;
        *(pServerData->xServerData.pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Proxy Registered");
    }

    return;
}

void GameServerNet_ServerModule::OnPvpManagerServerRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!INetModule::ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        std::shared_ptr<ProxyServerInfo> pServerData = mGameplayManagerMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ProxyServerInfo>(new ProxyServerInfo());
            mGameplayManagerMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->xServerData.nFD = sock;
        *(pServerData->xServerData.pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Gameplay Manager Registered");
    }

    return;
}

void GameServerNet_ServerModule::OnPvpManagerServerUnRegisteredProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        mGameplayManagerMap.RemoveElement(xData.server_id());

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Gameplay Manager Registered");
    }

    return;
}

void GameServerNet_ServerModule::OnRefreshPvpManagerServerInfoProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);
        std::shared_ptr<ProxyServerInfo> pServerData = mGameplayManagerMap.GetElement(xData.server_id());
        if (!pServerData) {
            pServerData = std::shared_ptr<ProxyServerInfo>(new ProxyServerInfo());
            mGameplayManagerMap.AddElement(xData.server_id(), pServerData);
        }

        pServerData->xServerData.nFD = sock;
        *(pServerData->xServerData.pData) = xData;

        m_log_->LogInfo(Guid(0, xData.server_id()), xData.server_name(), "Gameplay Manager Registered");
    }

    return;
}

void GameServerNet_ServerModule::SendMsgPBToProxy(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &self) {
    std::shared_ptr<ProxyBaseInfo> pData = mRoleBaseData.GetElement(self);
    if (pData) {
        std::shared_ptr<ProxyServerInfo> pProxyData = mProxyMap.GetElement(pData->proxy_id_);
        if (pProxyData) {
            m_net_->SendMsgPB(msg_id, xMsg, pProxyData->xServerData.nFD, pData->xClientID);
        }
    }
}

void GameServerNet_ServerModule::SendMsgToProxy(const uint16_t msg_id, const std::string &msg, const Guid &self) {
    std::shared_ptr<ProxyBaseInfo> pData = mRoleBaseData.GetElement(self);
    if (pData) {
        std::shared_ptr<ProxyServerInfo> pProxyData = mProxyMap.GetElement(pData->proxy_id_);
        if (pProxyData) {
            m_net_->SendMsg(msg_id, msg, pProxyData->xServerData.nFD, pData->xClientID);
        }
    }
}

// 发送给 Gameplay Manager 服务器
void GameServerNet_ServerModule::SendMsgPBToGameplayManager(const uint16_t msg_id, google::protobuf::Message &xMsg) {
    // 选择Gameplay转发表中的第一个Gameplay Manager进行发送,之后根据workload进行分配查找Gameplay Manager服务器
    ProxyServerInfo *pGameData = mGameplayManagerMap.FirstNude();
    if (pGameData) {
        dout << "发送给 Gameplay Manager 服务器: " << pGameData->xServerData.pData << std::endl;
        m_net_->SendMsgPB(msg_id, xMsg, pGameData->xServerData.nFD);
    } else {
        dout << "未找到 Gameplay Manager 服务器\n";
    }
}

void GameServerNet_ServerModule::SendMsgToGameplayManager(const uint16_t msg_id, const std::string &msg) {
    // 选择Gameplay转发表中的第一个Gameplay Manager进行发送
    ProxyServerInfo *pGameData = mGameplayManagerMap.FirstNude();
    if (pGameData) {
        m_net_->SendMsg(msg_id, msg, pGameData->xServerData.nFD);
    } else {
        dout << "未找到 Gameplay Manager 服务器\n";
    }
}

// 发送给Gameplay服务器
void GameServerNet_ServerModule::SendMsgPBToGameplay(const uint16_t msg_id, google::protobuf::Message &xMsg, const Guid &self) {
    ProxyServerInfo *pGameData = mGameplayManagerMap.FirstNude();
    if (pGameData) {
        dout << "发送给 Gameplay Manager 服务器: " << pGameData->xServerData.pData << std::endl;
        m_net_->SendMsgPB(msg_id, xMsg, pGameData->xServerData.nFD, self);
    } else {
        dout << "未找到 Gameplay Manager 服务器\n";
    }
}

void GameServerNet_ServerModule::SendMsgToGameplay(const uint16_t msg_id, const std::string &msg, const Guid &self) {
    ProxyServerInfo *pGameData = mGameplayManagerMap.FirstNude();
    if (pGameData) {
        dout << "发送给 Gameplay Manager 服务器: " << pGameData->xServerData.pData << std::endl;
        m_net_->SendMsg(msg_id, msg, pGameData->xServerData.nFD, self);
    } else {
        dout << "未找到 Gameplay Manager 服务器\n";
    }
}
// ---

void GameServerNet_ServerModule::SendGroupMsgPBToProxy(const uint16_t msg_id, google::protobuf::Message &xMsg, const int sceneID, const int groupID) {
    // care: batch
    DataList xList;
    if (m_kernel_->GetGroupObjectList(sceneID, groupID, xList, true)) {
        for (int i = 0; i < xList.GetCount(); ++i) {
            Guid xObject = xList.Object(i);
            this->SendMsgPBToProxy(msg_id, xMsg, xObject);
        }
    }
}

void GameServerNet_ServerModule::SendGroupMsgPBToProxy(const uint16_t msg_id, google::protobuf::Message &xMsg, const int sceneID, const int groupID,
                                                      const Guid exceptID) {
    DataList xList;
    if (m_kernel_->GetGroupObjectList(sceneID, groupID, xList, true)) {
        for (int i = 0; i < xList.GetCount(); ++i) {
            Guid xObject = xList.Object(i);
            if (xObject != exceptID) {
                this->SendMsgPBToProxy(msg_id, xMsg, xObject);
            }
        }
    }
}

void GameServerNet_ServerModule::SendGroupMsgPBToProxy(const uint16_t msg_id, const std::string &msg, const int sceneID, const int groupID) {
    // care: batch
    DataList xList;
    if (m_kernel_->GetGroupObjectList(sceneID, groupID, xList, true)) {
        for (int i = 0; i < xList.GetCount(); ++i) {
            Guid xObject = xList.Object(i);
            this->SendMsgToProxy(msg_id, msg, xObject);
        }
    }
}

void GameServerNet_ServerModule::SendGroupMsgPBToProxy(const uint16_t msg_id, const std::string &msg, const int sceneID, const int groupID, const Guid exceptID) {
    DataList xList;
    if (m_kernel_->GetGroupObjectList(sceneID, groupID, xList, true)) {
        for (int i = 0; i < xList.GetCount(); ++i) {
            Guid xObject = xList.Object(i);
            if (xObject != exceptID) {
                this->SendMsgToProxy(msg_id, msg, xObject);
            }
        }
    }
}

bool GameServerNet_ServerModule::AddPlayerProxyInfo(const Guid &roleID, const Guid &clientID, const int proxy_id) {
    if (proxy_id <= 0) {
        return false;
    }

    if (clientID.IsNull()) {
        return false;
    }

    std::shared_ptr<GameServerNet_ServerModule::ProxyBaseInfo> pBaseData = mRoleBaseData.GetElement(roleID);
    if (nullptr != pBaseData) {
        m_log_->LogError(clientID, "player is exist, cannot enter game", __FUNCTION__, __LINE__);
        return false;
    }

    std::shared_ptr<ProxyServerInfo> pServerData = mProxyMap.GetElement(proxy_id);
    if (nullptr == pServerData) {
        return false;
    }

    if (!pServerData->xRoleInfo.insert(std::make_pair(roleID, pServerData->xServerData.nFD)).second) {
        return false;
    }

    if (!mRoleBaseData.AddElement(roleID, std::shared_ptr<ProxyBaseInfo>(new ProxyBaseInfo(proxy_id, clientID)))) {
        pServerData->xRoleInfo.erase(roleID);
        return false;
    }

    return true;
}

/*
bool GameServerNet_ServerModule::AddPvpGateInfo(const Guid& clientID, const int gateID)
{
        if (gateID <= 0) {
                return false;
        }

        if (clientID.IsNull()) {
                return false;
        }

        std::shared_ptr<GateServerInfo> pServerData = mPvpManagerMap.GetElement(gateID);
        if (nullptr == pServerData) {
                return false;
        }

        if (!pServerData->xRoleInfo.insert(std::make_pair(clientID, pServerData->xServerData.nFD)).second) {
                return false;
        }

        return true;
}*/

bool GameServerNet_ServerModule::RemovePlayerProxyInfo(const Guid &roleID) {
    std::shared_ptr<ProxyBaseInfo> pBaseData = mRoleBaseData.GetElement(roleID);
    if (nullptr == pBaseData) {
        return false;
    }

    mRoleBaseData.RemoveElement(roleID);

    std::shared_ptr<ProxyServerInfo> pServerData = mProxyMap.GetElement(pBaseData->proxy_id_);
    if (nullptr == pServerData) {
        return false;
    }

    pServerData->xRoleInfo.erase(roleID);
    return true;
}

std::shared_ptr<IGameServerNet_ServerModule::ProxyBaseInfo> GameServerNet_ServerModule::GetPlayerProxyInfo(const Guid &roleID) {
    return mRoleBaseData.GetElement(roleID);
}

std::shared_ptr<IGameServerNet_ServerModule::ProxyServerInfo> GameServerNet_ServerModule::GetProxyServerInfo(const int proxy_id) {
    return mProxyMap.GetElement(proxy_id);
}

std::shared_ptr<IGameServerNet_ServerModule::ProxyServerInfo> GameServerNet_ServerModule::GetProxyServerInfoBySockIndex(const socket_t sock) {
    int gateID = -1;
    std::shared_ptr<ProxyServerInfo> pServerData = mProxyMap.First();
    while (pServerData) {
        if (sock == pServerData->xServerData.nFD) {
            gateID = pServerData->xServerData.pData->server_id();
            break;
        }

        pServerData = mProxyMap.Next();
    }

    if (gateID == -1) {
        return nullptr;
    }

    return pServerData;
}

void GameServerNet_ServerModule::OnTransWorld(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    std::string msgData;
    Guid nPlayer;
    int64_t nHasKey = 0;
    if (INetModule::ReceivePB(msg_id, msg, len, msgData, nPlayer)) {
        nHasKey = nPlayer.nData64;
    }

    m_net_client_->SendBySuitWithOutHead(ServerType::SQUICK_ST_WORLD, nHasKey, msg_id, std::string(msg, len));
}
} // namespace game::server