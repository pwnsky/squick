
#include "logic_module.h"
#include "plugin.h"

bool ProxyLogicModule::Start() { return true; }

bool ProxyLogicModule::Destory() { return true; }

bool ProxyLogicModule::Update() { return true; }

bool ProxyLogicModule::AfterStart() {
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();

    m_pNetModule->AddReceiveCallBack(SquickStruct::ProxyRPC::REQ_HEARTBEAT, this, &ProxyLogicModule::OnHeartbeat);

    return true;
}

void ProxyLogicModule::OnHeartbeat(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    std::string msgData(msg, len);
    m_pNetModule->SendMsgWithOutHead(SquickStruct::ProxyRPC::ACK_HEARTBEAT, msgData, sockIndex);

    // TODO improve performance
    NetObject *pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
    if (pNetObject) {
        const int gameID = pNetObject->GetGameID();
        m_pNetClientModule->SendByServerIDWithOutHead(gameID, msgID, msgData);
    }
}
