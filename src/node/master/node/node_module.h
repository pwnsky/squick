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
    map<int, ServerInfo>& GetServers();
};

} // namespace master::server