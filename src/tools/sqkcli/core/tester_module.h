#pragma once

#include "i_tester_module.h"
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

namespace tester::core {

enum class TestType {
    None = 0,
    Proxy = 1,
    All = 1000,
};

class TesterModule : public ITesterModule {

  public:
    TesterModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Destroy();
    virtual bool Update();

  protected:
    void TestProxyTransferSpeed_Init();
    void TestProxyTransferSpeed_Req();
    void TestProxyTransferSpeed_Ack(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnClientSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

  private:
    TestType test_type_ = TestType::None;
    bool is_connected_ = false;

    int test_req_index_ = 0;
    string test_req_data_;

    INT64 req_time_ = 0;

    int test_ack_index_ = 0;
    string test_ack_data_;

    bool is_hide_ = false;
    // IKe

    IElementModule *m_element_;
    IClassModule *m_class_;
    ILogModule *m_log_;
    INetModule *m_net_;
    INetClientModule *m_net_client_;
};

} // namespace tester::core