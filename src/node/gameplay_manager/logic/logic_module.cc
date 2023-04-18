
#include "logic_module.h"
#include "plugin.h"
namespace gameplay_manager::logic {
bool LogicModule::Start() { return true; }

bool LogicModule::Destory() { return true; }

bool LogicModule::Update() { return true; }

bool LogicModule::AfterStart() {
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_net_ = pm_->FindModule<INetModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];
            int nWebServerAppID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
            // webserver only run one instance in each server
            if (pm_->GetAppID() == nWebServerAppID) {
                public_port_ = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                public_ip_ = m_element_->GetPropertyString(strId, excel::Server::PublicIP());
                break;
            }
        }
    }

    // 来自客户端
    m_net_->AddReceiveCallBack(rpc::SERVER_HEARTBEAT, this, &LogicModule::OnLagTestProcess);

    // 来自Game 服务器
    m_net_client_->AddReceiveCallBack(ServerType::ST_GAME, rpc::REQ_GAMEPLAY_CREATE, this, &LogicModule::OnReqPvpInstanceCreate);

    return true;
}

void LogicModule::OnLagTestProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    std::string msgDatag(msg, len);
    m_net_->SendMsgWithOutHead(rpc::ServerRPC::SERVER_HEARTBEAT, msgDatag, sock);

    // TODO improve performance
    NetObject *pNetObject = m_net_->GetNet()->GetNetObject(sock);
    if (pNetObject) {
        const int gameID = pNetObject->GetGameID();
        // 避免不必要的开销，心跳包与PVP Manager进行跳动就行
        // m_net_client_->SendByServerIDWithOutHead(gameID, msg_id, msgDatag);
    }
}

int LogicModule::GetUnbindPort() {
    int seed = time(0);
    srand(seed);
    return 30000 + rand() % 10000;
}

// Game Server请求创建PVP服务器实例
void LogicModule::OnReqPvpInstanceCreate(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {

    Guid tmpID; // 服务端之间推送，ID值无效
    rpc::ReqGameplayCreate xMsg;
    if (!m_net_->ReceivePB(msg_id, msg, len, xMsg, tmpID)) {
        return;
    }

    dout << "Game Server 请求创建PVP实例 from " << xMsg.game_id() << " id: " << xMsg.id() << " key: " << xMsg.key() << std::endl;
    string cmd;
    // 为了测试方便，先暂时采用system来启动Gameplay服务器，后期采用docker进行管理 Gameplay 服务器
#if PLATFORM == PLATFORM_WIN
    cmd = "start gameplay/server.exe";
#else
    cmd = "bash gameplay/server.sh";
#endif
    cmd += " -instance_id=" + to_string(xMsg.id());
    cmd += " -instance_key=" + string(xMsg.key());
    cmd += " -game_id=" + std::to_string(xMsg.game_id());
    cmd += " -mip=" + public_ip_;                       // Gameplay Manager IP
    cmd += " -mport=" + std::to_string(public_port_);   // Gameplay Manager 端口
    cmd += " -ip=" + public_ip_;                        // Gameplay IP地址
    cmd += " -log";                                     // 打印日志
    cmd += " -port=" + std::to_string(GetUnbindPort()); // 端口
#if PLATFORM == PLATFORM_LINUX
    cmd += " &";
#endif
    dout << "执行: " << cmd << std::endl;
    system(cmd.c_str());
    dout << "执行命令结束" << std::endl;
}

} // namespace gameplay_manager::logic