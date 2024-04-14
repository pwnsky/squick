#include "plugin.h"

#include "node_module.h"
namespace proxy::node {

bool NodeModule::AfterStart() {
    m_logic_ = pm_->FindModule<logic::ILogicModule>();
    m_ws_ = pm_->FindModule<IWSModule>();

    Listen();

    node_info_.info->set_ws_port(pm_->GetArg("ws_port=", 10502));

    m_ws_->Listen(DEFAULT_NODE_MAX_SERVER_CONNECTION, pm_->GetArg("ws_port=", 10502), DEFAULT_NODE_CPUT_COUNT, DEFAULT_NET_SERVER_BUFFER_SIZE);
    m_ws_->AddEventCallBack(this, &NodeModule::OnWebSocketClientEvent);

    vector<int> node_types = { ServerType::ST_WORLD, ServerType::ST_LOGIN, ServerType::ST_PLAYER };
    AddNodesByType(node_types);
    return true;
}


bool NodeModule::Destroy() { return true; }

void NodeModule::OnClientConnected(socket_t sock) {  }

void NodeModule::OnClientDisconnected(socket_t sock) { m_logic_->OnClientDisconnected(sock); }

void NodeModule::OnWebSocketClientEvent(socket_t sock, const SQUICK_NET_EVENT eEvent, INet* pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        m_logic_->OnClientDisconnected(sock);
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        m_logic_->OnClientDisconnected(sock);
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        m_logic_->OnClientDisconnected(sock);
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        LOG_INFO("New websocket client connected, sock<%v>", sock);
    }
}
} // namespace proxy::server