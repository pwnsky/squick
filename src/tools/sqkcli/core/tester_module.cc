
#include "tester_module.h"
#include "plugin.h"
#include <squick/core/plugin_server.h>
namespace tester::core {
bool TesterModule::Start() {
    // PluginServer::FindParameterValue()

    std::cout << "Squick Tester\n";

    m_net_ = pm_->FindModule<INetModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    m_element_ = pm_->FindModule<IElementModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_net_client_ = pm_->FindModule<INetClientModule>();

    return true;
}

bool TesterModule::AfterStart() {

    string test_type = pm_->FindParameterValue("test=");
    if (pm_->FindParameterValue("hide=") == "true") {
        is_hide_ = true;
    }

    std::cout << test_type << std::endl;
    if (test_type == "proxy") {
        node_type_ = rpc::ST_PROXY;
        ProxyNodeTestInit();
    }else if (test_type == "player") {
        node_type_ = rpc::ST_PLAYER;
        PlayerNodeTestInit();
    }
    return true;
}

void TesterModule::ProxyNodeTestInit() {
    std::cout << "Test proxy node\n";
    m_net_client_->AddReceiveCallBack(rpc::ST_PROXY, rpc::ACK_TEST_PROXY, this, &TesterModule::TestProxyTransferSpeed_Ack);
    m_net_client_->AddEventCallBack(rpc::ST_PROXY, this, &TesterModule::OnClientSocketEvent);

    ConnectData s;
    s.id = 1; // Just for test
    s.type = rpc::ST_PROXY;
    s.ip = pm_->GetArg("ip=", "127.0.0.1");
    s.port = pm_->GetArg("port=", 10501);
    s.name = "test";
    s.buffer_size = 0;
    m_net_client_->AddNode(s);
}

void TesterModule::PlayerNodeTestInit() {
    std::cout << "Test player node\n";
    m_net_client_->AddReceiveCallBack(rpc::ST_PLAYER, rpc::IdTAckConfig, this, &TesterModule::TAckConfig);
    m_net_client_->AddEventCallBack(rpc::ST_PLAYER, this, &TesterModule::OnClientSocketEvent);

    ConnectData s;
    s.id = 1; // Just for test
    s.type = rpc::ST_PLAYER;
    s.ip = pm_->GetArg("ip=", "127.0.0.1");
    s.port = pm_->GetArg("port=", 10401);
    s.name = "player node test";
    s.buffer_size = 0;
    m_net_client_->AddNode(s);
}

void TesterModule::TestProxyTransferSpeed_Req() {
    rpc::Test test;
    test.set_index(test_req_index_);
    test.set_data(test_req_data_);
    test.set_req_time(SquickGetTimeMSEx());
    m_net_client_->SendPBToAllNodeByType(rpc::ST_PROXY, rpc::TestRPC::REQ_TEST_PROXY, test, 0);
    test_req_index_++;
    test_req_data_ = "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
}

void TesterModule::TestProxyTransferSpeed_Ack(const socket_t sock, const int msg_id, const char *msg, const uint32_t len) {
    uint64_t uid;
    rpc::Test ack;
    if (!INetModule::ReceivePB(msg_id, msg, len, ack, uid)) {
        std::cout << "Error\n";
    }

    INT64 now_time = SquickGetTimeMSEx();
    static INT64 last_ack_time = 0;
    static int last_index = 0;

    if (now_time - last_ack_time > 10000000) {

        if (!is_hide_)
            std::cout << "TransferSpeed:"
                      << "\n  req_times: " << ack.index() - last_index << " times/10 second \n  last_req_ack_time: " << (now_time - ack.req_time()) / 1000.0f
                      << " ms " << std::endl;

        last_index = ack.index();
        last_ack_time = now_time;
    }
}

void TesterModule::TReqConfig() {
    dout << "Okkkk\n";
    rpc::TReqConfig test;
    test.set_index(test_req_index_);
    test.set_data(test_req_data_);
    test.set_req_time(SquickGetTimeMSEx());
    m_net_client_->SendPBToAllNode(rpc::IdTReqConfig, test, 0);
    
    test_req_index_++;
    test_req_data_ = "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
}

void TesterModule::TAckConfig(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    uint64_t uid;
    rpc::TAckConfig ack;
    if (!INetModule::ReceivePB(msg_id, msg, len, ack, uid)) {
        std::cout << "Error\n";
    }

    INT64 now_time = SquickGetTimeMSEx();
    static INT64 last_ack_time = 0;
    static int last_index = 0;

    if (now_time - last_ack_time > 10000000) {

        if (!is_hide_)
            std::cout << "TestConfig:"
            << "\n  req_times: " << ack.index() - last_index << " times/10 second \n  last_req_ack_time: " << (now_time - ack.req_time()) / 1000.0f
            << " ms " << std::endl;

        last_index = ack.index();
        last_ack_time = now_time;
    }
}

void TesterModule::OnClientSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
    } else if (eEvent & SQUICK_NET_EVENT_ERROR) {
    } else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
    } else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        std::cout << "Start to test ...\n";
        is_connected_ = true;
    }
}

bool TesterModule::Destroy() { return true; }

bool TesterModule::Update() {
    if (is_connected_) {
        switch (node_type_) {
        case rpc::ST_PROXY: {
            TestProxyTransferSpeed_Req();
        } break;
        case rpc::ST_PLAYER: {
            TReqConfig();
        }break;
        
        }
    }
    return true;
}

} // namespace tester::core
