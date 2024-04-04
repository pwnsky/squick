
#include "tester_module.h"
#include "plugin.h"
#include <squick/core/plugin_server.h>
namespace tester::core {
bool TesterModule::Start() {
    //PluginServer::FindParameterValue()

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
        test_type_ = TestType::Proxy;
        TestProxyTransferSpeed_Init();
    }
    else {
        test_type_ = TestType::All;
    }
    return true;
}


void TesterModule::TestProxyTransferSpeed_Init() {
    std::cout << "Test proxy Transfer speed!\n";

    m_net_client_->AddReceiveCallBack(ServerType::ST_PROXY, rpc::TestRPC::ACK_TEST_PROXY, this, &TesterModule::TestProxyTransferSpeed_Ack);

    AddServer(ServerType::ST_PROXY);
    
}

void TesterModule::TestProxyTransferSpeed_Req() {
    rpc::Test test;
    test.set_index(test_req_index_);
    test.set_data(test_req_data_);
    test.set_req_time(SquickGetTimeMSEx());
    m_net_client_->SendPBToAllNodeByType(ServerType::ST_PROXY, rpc::TestRPC::REQ_TEST_PROXY, test, 0);
    test_req_index_++;
    test_req_data_ = "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
}

void TesterModule::TestProxyTransferSpeed_Ack(const socket_t sock, const int msg_id, const char* msg, const uint32_t len) {
    uint64_t uid;
    rpc::Test ack;
    if (!INetModule::ReceivePB(msg_id, msg, len, ack, uid)) {
        std::cout << "Error\n";
    }

    INT64 now_time = SquickGetTimeMSEx();
    static INT64 last_ack_time = 0;
    static int last_index = 0;
    
    if (now_time - last_ack_time > 10000000) {

        if(!is_hide_)
            std::cout << "Test:" << "\n  req_times: " << ack.index() - last_index  << " times/10 second \n  last_req_ack_time: " << (now_time - ack.req_time()) / 1000.0f  << " ms " << std::endl;
        
        last_index = ack.index();
        last_ack_time = now_time;

    }
}

bool TesterModule::AddServer(ServerType type) {
    m_net_client_->AddEventCallBack(type, this, &TesterModule::OnClientSocketEvent);
    m_net_client_->ExpandBufferSize();

    /*
    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string>& strIdList = xLogicClass->GetIDList();

        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string& strId = strIdList[i];

            const int server_type = m_element_->GetPropertyInt32(strId, excel::Server::Type());
            
            if (server_type == type) {
                const int server_id = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());
                const int nPort = m_element_->GetPropertyInt32(strId, excel::Server::Port());
                const std::string& name = m_element_->GetPropertyString(strId, excel::Server::ID());
                const std::string& ip = m_element_->GetPropertyString(strId, excel::Server::IP());
                ConnectData s;
                s.id = server_id;
                s.type = (ServerType)server_type;
                s.ip = ip;
                s.port = nPort;
                s.name = strId;
                m_net_client_->AddNode(s);
                return true;
            }
        }
    }*/
    return false;
}

void TesterModule::OnClientSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet* pNet) {
    if (eEvent & SQUICK_NET_EVENT_EOF) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_EOF", __FUNCTION__, __LINE__);
    }
    else if (eEvent & SQUICK_NET_EVENT_ERROR) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_ERROR", __FUNCTION__, __LINE__);
    }
    else if (eEvent & SQUICK_NET_EVENT_TIMEOUT) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_TIMEOUT", __FUNCTION__, __LINE__);
    }
    else if (eEvent & SQUICK_NET_EVENT_CONNECTED) {
        m_log_->LogInfo(Guid(0, sock), "SQUICK_NET_EVENT_CONNECTED", __FUNCTION__, __LINE__);

        std::cout << "Start to test ...\n";
        is_connected_ = true;
    }
}

bool TesterModule::Destory() {
    return true;
}


bool TesterModule::Update() {
    if (is_connected_) {
        switch (test_type_) {
        case TestType::Proxy: {
            TestProxyTransferSpeed_Req();
        }break;
        }
    }
    return true;
}

} // namespace tester::core
