

#include "node_module.h"
#include <squick/plugin/lua/export.h>
#include <struct/struct.h>
#include <third_party/nlohmann/json.hpp>
namespace gameplay_manager::node {


bool NodeModule::Destroy() { return true; }

bool NodeModule::AfterStart() {
    // PVP校验连接Key,校验成功后，之后的数据直接转发到Game服务器上
    //m_net_->AddReceiveCallBack(rpc::REQ_GAMEPLAY_CONNECT_GAME_SERVER, this, &NodeModule::OnReqConnect);
    //m_net_->AddReceiveCallBack(this, &NodeModule::OnOtherMessage);
    Listen();
    //AddServer(ServerType::ST);
    return true;
}

// Gameplay服务器请求连接 Gameplay manager服务器
void NodeModule::OnReqConnect(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

// 来自PVP服务器消息，根据校验连接成功PVP服务器数据包转发达到Game服务器
//void NodeModule::OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

// PVP 服务端断开连接
void NodeModule::OnClientDisconnect(const socket_t sock) {}

// 转发给PVP服务器
bool NodeModule::Transport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) { return true; }


} // namespace gameplay_manager::node