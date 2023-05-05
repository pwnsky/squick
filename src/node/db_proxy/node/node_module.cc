#include <squick/struct/struct.h>

//#include "../logic/player_redis_module.h"
#include "node_module.h"

namespace db_proxy::node {

bool NodeModule::AfterStart() {
    m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_PLAYER_DATA_LOAD, this, &NodeModule::OnLoadRoleDataProcess);
    m_net_->AddReceiveCallBack(rpc::DbProxyRPC::REQ_PLAYER_DATA_SAVE, this, &NodeModule::OnSaveRoleDataProcess);


    m_pAccountRedisModule = pm_->FindModule<IAccountRedisModule>();
    m_pPlayerRedisModule = pm_->FindModule<IPlayerRedisModule>();
    
    Listen();
    return true;
}

bool NodeModule::Destory() { return true; }


void NodeModule::OnClientDisconnect(const socket_t sock) {}

void NodeModule::OnClientConnected(const socket_t sock) {}

void NodeModule::OnLoadRoleDataProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    Guid guid;
    rpc::ReqEnter req;
    if (!m_net_->ReceivePB(msg_id, msg, len, req, guid)) {
        return;
    }

    dout << guid.ToString() << "请求加载角色数据\n";

    // Guid roleID = INetModule::ProtobufToStruct(xMsg.object());
    Guid xID = m_kernel_->CreateGUID();
    rpc::DbPlayerData ack;
    ack.mutable_object()->CopyFrom(INetModule::StructToProtobuf(xID));
    ack.set_account(req.account());
    ack.mutable_guid()->CopyFrom(req.guid());
    // PlayerRedisModule* pPlayerRedisModule = (PlayerRedisModule*)m_pPlayerRedisModule;
    // pPlayerRedisModule->LoadPlayerData(roleID, xPlayerData);

    m_net_->SendMsgPB(rpc::DbProxyRPC::ACK_PLAYER_DATA_LOAD, ack, sock, guid);
}

void NodeModule::OnSaveRoleDataProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    /*
    Guid clientID;
    rpc::RoleDataPack xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, clientID))
    {
            return;
    }

    Guid roleID = INetModule::ProtobufToStruct(xMsg.id());

    PlayerRedisModule* pPlayerRedisModule = (PlayerRedisModule*)m_pPlayerRedisModule;
    pPlayerRedisModule->SavePlayerData(roleID, xMsg);

    */
}

} // namespace db_proxy::server