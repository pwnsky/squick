#pragma once
#include "i_node_module.h"
#include <set>

namespace master::node {
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager *p) { pm_ = p; }
    virtual ~NodeModule();

    virtual bool Destroy();
    virtual bool AfterStart();

    map<int, ServerInfo> &GetAllNodes();

    void OnNNtfNodeReport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNReqNodeRegister(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNReqNodeUnregistered(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNReqMinWorkNodeInfo(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNNtfNodeMsgForward(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    int GetLoadBanlanceNode(ServerType type);

    void AddSubscribeNode(int new_node_id, vector<int> types);
    void NtfSubscribNode(int new_node_id);

    bool SendPBByID(const int node_id, const uint16_t msg_id, const google::protobuf::Message &pb);

    inline bool IsHaveThisType(const vector<int> &types, int type) {
        for (auto t : types) {
            if (t == type) {
                return true;
            }
        }
        return false;
    }

  private:
    // 服务表
    map<int, ServerInfo> node_map_;                // ikey -> node id, value -> ServerInfo
    std::map<int, std::set<int>> nodes_subscribe_; // key -> node type, value -> node ids;
};

} // namespace master::node