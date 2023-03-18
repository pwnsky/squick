
#include "logic_module.h"
#include "plugin.h"
namespace gameplay_manager::logic {
bool LogicModule::Start() { return true; }

bool LogicModule::Destory() { return true; }

bool LogicModule::Update() { return true; }

bool LogicModule::AfterStart() {
    m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
    m_pClassModule = pPluginManager->FindModule<IClassModule>();
    m_pNetModule = pPluginManager->FindModule<INetModule>();
    m_pNetClientModule = pPluginManager->FindModule<INetClientModule>();

    // 来自客户端
    m_pNetModule->AddReceiveCallBack(SquickStruct::STS_HEART_BEAT, this, &LogicModule::OnLagTestProcess);

    // 来自Game 服务器
    m_pNetClientModule->AddReceiveCallBack(SQUICK_SERVER_TYPES::SQUICK_ST_GAME, SquickStruct::REQ_GAMEPLAY_CREATE, this, &LogicModule::OnReqPvpInstanceCreate);

    return true;
}

void LogicModule::OnLagTestProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {
    std::string msgDatag(msg, len);
    m_pNetModule->SendMsgWithOutHead(SquickStruct::ServerRPC::STS_HEART_BEAT, msgDatag, sockIndex);

    // TODO improve performance
    NetObject *pNetObject = m_pNetModule->GetNet()->GetNetObject(sockIndex);
    if (pNetObject) {
        const int gameID = pNetObject->GetGameID();
        // 避免不必要的开销，心跳包与PVP Manager进行跳动就行
        // m_pNetClientModule->SendByServerIDWithOutHead(gameID, msgID, msgDatag);
    }
}

int LogicModule::GetUnbindPort() {
    int seed = time(0);
    srand(seed);
    return 30000 + rand() % 10000;
}

// Game Server请求创建PVP服务器实例
void LogicModule::OnReqPvpInstanceCreate(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len) {

    Guid tmpID; // 服务端之间推送，ID值无效
    SquickStruct::ReqGameplayCreate xMsg;
    if (!m_pNetModule->ReceivePB(msgID, msg, len, xMsg, tmpID)) {
        return;
    }

    dout << "Game Server 请求创建PVP实例 from " << xMsg.game_id() << std::endl;
    string cmd;
    // 为了测试方便，先暂时采用system来启动PVP服务器，后期采用docker进行管理 PVP 服务器
#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
    cmd = "start game/Action.exe";
#else
    cmd = "bash game/ActionServer.sh";
#endif
    cmd += " -instance_id=" + xMsg.id();
    cmd += " -instance_key=" + xMsg.key();
    cmd += " -game_id=" + std::to_string(xMsg.game_id());
    cmd += " -room_id=" + std::to_string(xMsg.room_id());
    cmd += " -mip=" + string("\"127.0.0.1\"");          // PVP Manager IP
    cmd += " -mport=" + string("20001");                // PVP Manager 端口
    cmd += " -ip=" + string("\"127.0.0.1\"");           // PVP IP地址
    cmd += " -log";                                     // 打印日志
    cmd += " -port=" + std::to_string(GetUnbindPort()); // 端口
#if SQUICK_PLATFORM == SQUICK_PLATFORM_LINUX
    cmd += " &";
#endif
    dout << "执行: " << cmd << std::endl;
    system(cmd.c_str());
    dout << "执行命令结束" << std::endl;
}

} // namespace gameplay_manager::logic