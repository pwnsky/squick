#include "login_module.h"
#include "plugin.h"

namespace proxy::client {
bool LoginModule::Start() {
    BaseStart();
    return true;
}

bool LoginModule::AfterStart() {
    m_net_client_->AddReceiveCallBack(ServerType::ST_LOGIN, rpc::LoginRPC::ACK_PROXY_CONNECT_VERIFY, this, &LoginModule::OnAckProxyConnectVerify);
    AddServer(ServerType::ST_LOGIN);
    return true;
}

bool LoginModule::Destory() { return true; }

bool LoginModule::Update() { return true; }

bool LoginModule::OnReqProxyConnectVerify(INT64 session, const std::string &guid, const std::string &key) {
    // dout << "请求验证\n";
    rpc::ReqConnectProxyVerify req;
    req.set_session(session);
    req.set_key(key);
    req.set_guid(guid);
    m_net_client_->SendToServerByPB(login_id_, rpc::LoginRPC::REQ_PROXY_CONNECT_VERIFY, req);
    return true;
}

void LoginModule::OnAckProxyConnectVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    m_logic_->OnAckConnectVerify(msg_id, msg, len);
    return;
}

} // namespace proxy::client