
#include "logic_module.h"
#include "plugin.h"

namespace login::logic {
bool LogicModule::Start() {
    m_net_ = pm_->FindModule<INetModule>();
    m_redis_ = pm_->FindModule<redis::IRedisModule>();
    return true;
}

bool LogicModule::AfterStart() {
    m_net_->AddReceiveCallBack(rpc::LoginRPC::REQ_PROXY_CONNECT_VERIFY, this, &LogicModule::OnConnectProxyVerify);
    return true;
}

bool LogicModule::Destory() { return true; }

void LogicModule::OnConnectProxyVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    dout << "代理服务器连接验证\n";
    Guid tmp;
    rpc::ReqConnectProxyVerify req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, tmp)) {
        return;
    }
    string proxy_key, proxy_limit_time, world_id, account;
    m_redis_->HashGet(req.guid(), "proxy_key", proxy_key);
    m_redis_->HashGet(req.guid(), "proxy_limit_time", proxy_limit_time);
    m_redis_->HashGet(req.guid(), "world_id", world_id);
    m_redis_->HashGet(req.guid(), "account", account);

    rpc::AckConnectProxyVerify ack;
    if (!proxy_key.empty() && !req.key().empty() && proxy_key == req.key()) {
        ack.set_code(0);
        ack.set_session(req.session());
        ack.set_world_id(atoi(world_id.c_str()));
        ack.set_account(account);
    } else {
        ack.set_code(1);
    }
    m_net_->SendMsgPB(rpc::LoginRPC::ACK_PROXY_CONNECT_VERIFY, ack, sock);
}

bool LogicModule::ReadyUpdate() { return true; }

bool LogicModule::Update() { return true; }

} // namespace login::logic