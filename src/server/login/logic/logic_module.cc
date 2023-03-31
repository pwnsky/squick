
#include "logic_module.h"
#include "plugin.h"

namespace login::logic {
bool LogicModule::Start() {
    m_net_ = pm_->FindModule<INetModule>();

    return true;
}

bool LogicModule::AfterStart() {
    m_net_->AddReceiveCallBack(rpc::GameLobbyRPC::REQ_ENTER, this, &LogicModule::OnConnectProxyVerify);
    return true;
}

bool LogicModule::Destory() { return true; }

void LogicModule::OnConnectProxyVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "代理服务器连接验证\n";
    //rpc::AckConnectProxy
}

bool LogicModule::ReadyUpdate() {
    return true;
}

bool LogicModule::Update() { return true; }



} // namespace login::logic