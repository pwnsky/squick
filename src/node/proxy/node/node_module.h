#pragma once
#include "i_node_module.h"
#include <node/proxy/logic/i_logic_module.h>
namespace proxy::node {
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager* p) { pm_ = p; is_update_ = true; }
    virtual bool Destory();
    virtual bool AfterStart();
    virtual void OnClientDisconnect(socket_t sock) override;
    virtual void OnClientConnected(socket_t sock) override;

    virtual bool OnReqProxyConnectVerify(INT64 session, const std::string& guid, const std::string& key) override;
    virtual int GetLoadBanlanceNode(ServerType type) override { return INodeBaseModule::GetLoadBanlanceNode(type); }
private:
    void OnAckProxyConnectVerify(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    // From lobby
    void Transport(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void PlayerBindEvent(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
  protected:
    logic::ILogicModule *m_logic_;
};

} // namespace proxy::node