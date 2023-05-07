#pragma once
#include "../logic/i_logic_module.h"
#include "i_node_module.h"

namespace login::node {
class NodeModule : public INodeModule {

  public:
      NodeModule(IPluginManager *p) { pm_ = p; }

    virtual bool Destory();
    virtual bool BeforeDestory();
    virtual bool AfterStart();

  protected:
    virtual void OnClientDisconnect(const socket_t sock) override;
    virtual void OnClientConnected(const socket_t sock) override;
    void InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    map<int, rpc::Server>& GetServers() override;
  private:
    map<int, rpc::Server> servers_;
};

} // namespace login::server