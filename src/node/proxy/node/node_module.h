#pragma once
#include "i_node_module.h"
#include <node/proxy/logic/i_logic_module.h>
namespace proxy::node {
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager* p) { pm_ = p; is_update_ = true; }
    virtual bool Destory();
    virtual bool AfterStart();
    virtual void OnClientConnected(socket_t sock) override;
    virtual void OnClientDisconnected(socket_t sock) override;
    
private:
    void OnWebSocketClientEvent(socket_t sock, const SQUICK_NET_EVENT eEvent, INet* pNet);
private:
    logic::ILogicModule *m_logic_;
    IWSModule* m_ws_;
};
} // namespace proxy::node