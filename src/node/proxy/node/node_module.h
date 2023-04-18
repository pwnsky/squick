#pragma once
#include "i_server_module.h"
#include <server/proxy/logic/i_logic_module.h>
namespace proxy::node {
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();
    virtual void OnClientDisconnect(socket_t sock) override;
    virtual void OnClientConnected(socket_t sock) override;

  protected:
    logic::ILogicModule *m_logic_;
};

} // namespace proxy::server