#pragma once
#include "i_node_module.h"
namespace master::node {
class NodeModule : public INodeModule {
  public:
      NodeModule(IPluginManager *p) {
        pm_ = p;
    }
    virtual ~NodeModule();

    virtual bool Destory();
    virtual bool AfterStart();

    virtual void LogReceive(const char *str) {}
    virtual void LogSend(const char *str) {}
    virtual std::string GetServersStatus();

  protected:
    void OnHeartBeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
};

} // namespace master::server