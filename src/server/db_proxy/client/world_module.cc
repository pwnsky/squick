

#include "world_module.h"
#include "plugin.h"
#include <squick/core/data_list.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/struct/struct.h>

bool DBToWorldModule::Start() {
    m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pElementModule = pPluginManager->FindModule<IElementModule>();
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    m_pSecurityModule = pPluginManager->FindModule<ISecurityModule>();

    return true;
}

bool DBToWorldModule::Destory() { return true; }

bool DBToWorldModule::AfterStart() {
    m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, this, &DBToWorldModule::InvalidMessage);

    m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::STS_NET_INFO, this, &DBToWorldModule::OnServerInfoProcess);

    m_pNetClientModule->AddEventCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, this, &DBToWorldModule::OnSocketMSEvent);
    m_pNetClientModule->ExpandBufferSize();

    SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        const int nCurAppID = pPluginManager->GetAppID();

        std::vector<std::string>::const_iterator itr = std::find_if(strIdList.begin(), strIdList.end(), [&](const std::string &strConfigId) {
            return nCurAppID == m_pElementModule->GetPropertyInt32(strConfigId, excel::Server::ServerID());
        });

        if (strIdList.end() == itr) {
            std::ostringstream strLog;
            strLog << "Cannot find current server, AppID = " << nCurAppID;
            m_pLogModule->LogError(NULL_OBJECT, strLog, __FILE__, __LINE__);
            NFASSERT(-1, "Cannot find current server", __FILE__, __FUNCTION__);
            exit(0);
        }

        const int nCurArea = m_pElementModule->GetPropertyInt32(*itr, excel::Server::Area());

        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
            const int nServerArea = m_pElementModule->GetPropertyInt32(strId, excel::Server::Area());
            if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_WORLD && nServerArea == nCurArea) {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());
                const std::string &name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

                ConnectData xServerData;

                xServerData.nGameID = serverID;
                xServerData.eServerType = (SQUICK_SERVER_TYPES)serverType;
                xServerData.ip = ip;
                xServerData.nPort = nPort;
                xServerData.name = strId;

                m_pNetClientModule->AddServer(xServerData);
            }
        }
    }

    return true;
}

bool DBToWorldModule::Update() {
    // SQUICK_WILL_DO
    // ServerReport();
    return true;
}

void DBToWorldModule::Register(INet *pNet) {
    SQUICK_SHARE_PTR<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
            if (serverType == SQUICK_SERVER_TYPES::SQUICK_ST_DB_PROXY && pPluginManager->GetAppID() == serverID) {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const int nCpus = m_pElementModule->GetPropertyInt32(strId, excel::Server::CpuCount());
                const std::string &name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

                SquickStruct::ServerInfoReportList xMsg;
                SquickStruct::ServerInfoReport *pData = xMsg.add_server_list();

                pData->set_server_id(serverID);
                pData->set_server_name(strId);
                pData->set_server_cur_count(0);
                pData->set_server_ip(ip);
                pData->set_server_port(nPort);
                pData->set_server_max_online(maxConnect);
                pData->set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
                pData->set_server_type(serverType);

                SQUICK_SHARE_PTR<ConnectData> pServerData = m_pNetClientModule->GetServerNetInfo(pNet);
                if (pServerData) {
                    int nTargetID = pServerData->nGameID;
                    m_pNetClientModule->SendToServerByPB(nTargetID, SquickStruct::ServerRPC::DB_TO_WORLD_REGISTERED, xMsg);

                    m_pLogModule->LogInfo(Guid(0, pData->server_id()), pData->server_name(), "Register");
                }
            }
        }
    }
}

void DBToWorldModule::ServerReport() {
    if (mLastReportTime + 10 > pPluginManager->GetNowTime()) {
        return;
    }
    mLastReportTime = pPluginManager->GetNowTime();
    std::shared_ptr<IClass> xLogicClass = m_pClassModule->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            const int serverType = m_pElementModule->GetPropertyInt32(strId, excel::Server::Type());
            const int serverID = m_pElementModule->GetPropertyInt32(strId, excel::Server::ServerID());
            if (pPluginManager->GetAppID() == serverID) {
                const int nPort = m_pElementModule->GetPropertyInt32(strId, excel::Server::Port());
                const int maxConnect = m_pElementModule->GetPropertyInt32(strId, excel::Server::MaxOnline());
                const std::string &name = m_pElementModule->GetPropertyString(strId, excel::Server::ID());
                const std::string &ip = m_pElementModule->GetPropertyString(strId, excel::Server::IP());

                SquickStruct::ServerInfoReport reqMsg;

                reqMsg.set_server_id(serverID);
                reqMsg.set_server_name(strId);
                reqMsg.set_server_cur_count(0);
                reqMsg.set_server_ip(ip);
                reqMsg.set_server_port(nPort);
                reqMsg.set_server_max_online(maxConnect);
                reqMsg.set_server_state(SquickStruct::ServerState::SERVER_NORMAL);
                reqMsg.set_server_type(serverType);

                m_pNetClientModule->SendToAllServerByPB(SQUICK_SERVER_TYPES::SQUICK_ST_WORLD, SquickStruct::STS_SERVER_REPORT, reqMsg, Guid());
            }
        }
    }
}

void DBToWorldModule::InvalidMessage(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    printf("Net || umsgID=%d\n", msgID);
}

void DBToWorldModule::OnSocketMSEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_EOF Connection closed", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_ERROR Got an error on the connection", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_TIMEOUT read timeout", __FUNCTION__, __LINE__);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_pLogModule->LogInfo(Guid(0, sockIndex), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
        Register(pNet);
    }
}

void DBToWorldModule::OnClientDisconnect(const SQUICK_SOCKET nAddress) {}

void DBToWorldModule::OnClientConnected(const SQUICK_SOCKET nAddress) {}

bool DBToWorldModule::BeforeDestory() { return true; }

void DBToWorldModule::LogServerInfo(const std::string &strServerInfo) { m_pLogModule->LogInfo(Guid(), strServerInfo, ""); }

void DBToWorldModule::OnServerInfoProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    Guid nPlayerID;
    SquickStruct::ServerInfoReportList xMsg;
    if (!INetModule::ReceivePB(msgID, msg, len, xMsg, nPlayerID)) {
        return;
    }

    for (int i = 0; i < xMsg.server_list_size(); ++i) {
        const SquickStruct::ServerInfoReport &xData = xMsg.server_list(i);

        // type
        ConnectData xServerData;

        xServerData.nGameID = xData.server_id();
        xServerData.ip = xData.server_ip();
        xServerData.nPort = xData.server_port();
        xServerData.name = xData.server_name();
        xServerData.nWorkLoad = xData.server_cur_count();
        xServerData.eServerType = (SQUICK_SERVER_TYPES)xData.server_type();

        if (SQUICK_SERVER_TYPES::SQUICK_ST_WORLD == xServerData.eServerType) {
            m_pNetClientModule->AddServer(xServerData);
        }
    }
}
