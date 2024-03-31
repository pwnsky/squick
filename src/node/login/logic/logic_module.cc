
#include "logic_module.h"
#include "plugin.h"

namespace login::logic {
bool LogicModule::Start() {
    m_net_ = pm_->FindModule<INetModule>();
    return true;
}

bool LogicModule::AfterStart() {
    m_net_->AddReceiveCallBack(rpc::LoginRPC::REQ_PROXY_CONNECT_VERIFY, this, &LogicModule::OnConnectProxyVerify);
    return true;
}

bool LogicModule::Destory() { return true; }

void LogicModule::OnConnectProxyVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    string tmp;
    rpc::ReqConnectProxyVerify req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, tmp)) {
        return;
    }
    // to do, auth from db

    rpc::AckConnectProxyVerify ack;
    ack.set_code(0);
    ack.set_session(req.session());
    ack.set_world_id(0);
    ack.set_account(req.guid());
    m_net_->SendMsgPB(rpc::LoginRPC::ACK_PROXY_CONNECT_VERIFY, ack, sock);
}

bool LogicModule::ReadyUpdate() { return true; }

bool LogicModule::Update() { return true; }

} // namespace login::logic