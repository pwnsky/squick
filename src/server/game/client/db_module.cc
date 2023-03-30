

#include "db_module.h"
#include "plugin.h"
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/struct/struct.h>

bool GameServerToDBModule::Start() {
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_pGameServerNet_ServerModule = pm_->FindModule<IGameServerNet_ServerModule>();

    return true;
}

bool GameServerToDBModule::Destory() { return true; }

bool GameServerToDBModule::Update() { return true; }

bool GameServerToDBModule::AfterStart() {
    // m_net_client_->AddReceiveCallBack(ServerType::SQUICK_ST_WORLD, this, &GameServerToDBModule::TransPBToProxy);
    // m_net_client_->AddEventCallBack(ServerType::SQUICK_ST_WORLD, this, &GameServerToDBModule::OnSocketWSEvent);

    m_net_client_->ExpandBufferSize();

    return true;
}

void GameServerToDBModule::OnSocketWSEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED connected success", __FUNCTION__, __LINE__);
    }
}

void GameServerToDBModule::TransPBToProxy(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    m_net_client_->SendBySuitWithOutHead(ServerType::SQUICK_ST_DB_PROXY, sock, msg_id, std::string(msg, len));

    return;
}

void GameServerToDBModule::TransmitToDB(const int nHashKey, const int msg_id, const google::protobuf::Message &xData) {
    m_net_client_->SendSuitByPB(ServerType::SQUICK_ST_DB_PROXY, nHashKey, msg_id, xData);
}