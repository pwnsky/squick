#include "rpc_module.h"

namespace tutorial {
bool SimpleModule::Start() {
    m_net_client_ = pm_->FindModule<INetClientModule>();
    m_net_client_->AddReceiveCallBack(ServerType::ST_GAME, 1123, this, &SimpleModule::OnRecivedMsg);
    dout << "RPC Start ...\n";
    return true;
}

bool SimpleModule::AfterStart() { return true; }

// Update
bool SimpleModule::Update() { return true; }

void SimpleModule::OnRecivedMsg(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {}

} // namespace tutorial