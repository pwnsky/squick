#pragma once
#include "i_node_module.h"
#include <set>

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

    void OnNnNtfNodeReport(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnNnReqNodeRegister(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnNnReqNodeUnregistered(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnNnReqMinWorkNodeInfo(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);

    int GetLoadBanlanceNode(ServerType type);

    void AddSubscribeNode(int new_node_id, vector<int> types);
    void NtfSubscribNode(int new_node_id);

    bool SendPBByID(const int node_id, const uint16_t msg_id, const google::protobuf::Message& pb);

    inline bool IsHaveThisType(const vector<int>& types, int type) {
        for (auto t : types) {
            if (t == type) {
                return true;
            }
        }
        return false;
    }

private:
    // 服务表
    map<int, ServerInfo> node_map_;
    std::map<int, std::set<int>> nodes_subscribe_; // key -> node type, value -> node ids;
};

} // namespace master::server