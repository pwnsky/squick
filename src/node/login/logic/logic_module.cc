
#include "logic_module.h"
#include "plugin.h"

namespace login::logic {
bool LogicModule::Start() {
    m_net_ = pm_->FindModule<INetModule>();
    return true;
}

bool LogicModule::AfterStart() {
    m_net_->AddReceiveCallBack(rpc::NLoginRPC::NREQ_PROXY_CONNECT_VERIFY, this, &LogicModule::OnConnectProxyVerify);
    return true;
}

bool LogicModule::Destory() { return true; }

void LogicModule::OnConnectProxyVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    uint64_t uid;
    rpc::NReqConnectProxyVerify req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, uid)) {
        return;
    }
    // to do, auth from db

    rpc::NAckConnectProxyVerify ack;
    ack.set_code(0);
    ack.set_session(req.session());
    ack.set_area_id(0);
    ack.set_account_id(req.account_id());
    m_net_->SendMsgPB(rpc::NLoginRPC::NACK_PROXY_CONNECT_VERIFY, ack, sock);
}

bool LogicModule::ReadyUpdate() { return true; }

bool LogicModule::Update() { return true; }

} // namespace login::logic