

#include "server_module.h"
#include <squick/plugin/lua/export.h>
#include <squick/struct/struct.h>
#include <third_party/nlohmann/json.hpp>
namespace gameplay_manager::server {
bool ServerModule::Start() {
    this->pm_->SetAppType(ServerType::ST_GAMEPLAY_MANAGER);
    m_net_ = pm_->FindModule<INetModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_security_ = pm_->FindModule<ISecurityModule>();
    m_pWsModule = pm_->FindModule<IWSModule>();
    return true;
}

bool ServerModule::Destory() { return true; }

bool ServerModule::AfterStart() {
    // PVP校验连接Key,校验成功后，之后的数据直接转发到Game服务器上
    m_net_->AddReceiveCallBack(rpc::REQ_GAMEPLAY_CONNECT_GAME_SERVER, this, &ServerModule::OnReqConnect);
    m_net_->AddReceiveCallBack(this, &ServerModule::OnOtherMessage);
    Listen();
    return true;
}

// Gameplay服务器请求连接 Gameplay manager服务器
void ServerModule::OnReqConnect(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

// 来自PVP服务器消息，根据校验连接成功PVP服务器数据包转发达到Game服务器
void ServerModule::OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

// PVP 服务端断开连接
void ServerModule::OnClientDisconnect(const socket_t sock) {}

// 转发给PVP服务器
bool ServerModule::Transport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { return true; }

bool ServerModule::Update() { return true; }

} // namespace gameplay_manager::server