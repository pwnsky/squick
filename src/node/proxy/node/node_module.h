#pragma once
#include "i_node_module.h"
#include <node/proxy/logic/i_logic_module.h>
namespace proxy::node {
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager *p) { pm_ = p; }
    virtual bool Destory();
    virtual bool AfterStart();
    virtual void OnClientDisconnect(socket_t sock) override;
    virtual void OnClientConnected(socket_t sock) override;

    virtual bool OnReqProxyConnectVerify(INT64 session, const std::string& guid, const std::string& key) override;

private:
    void OnAckProxyConnectVerify(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    // From lobby
    void Transport(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnAckEnter(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
  protected:
    logic::ILogicModule *m_logic_;
};

} // namespace proxy::node