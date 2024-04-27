#pragma once

#include "i_logic_module.h"
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/node/export.h>
#include <struct/struct.h>
namespace master::logic {
class LogicModule : public ILogicModule {

  public:
    LogicModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }

    virtual bool Start() override;
    virtual bool AfterStart() override;
    virtual bool Destroy() override;
    virtual bool Update() override;

    void OnNNtfNodeReport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNReqNodeRegister(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNReqNodeUnregistered(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNReqMinWorkNodeInfo(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNNtfNodeMsgForward(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnNReqAllNodesInfo(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    int GetLoadBanlanceNode(int type);

    void AddSubscribeNode(int new_node_id, vector<int> types);
    void NtfSubscribNode(int new_node_id);

    bool SendPBByID(const int node_id, const uint16_t msg_id, const google::protobuf::Message &pb);
    void UpdateStatus();

    inline bool IsHaveThisType(const vector<int> &types, int type) {
        for (auto t : types) {
            if (t == type) {
                return true;
            }
        }
        return false;
    }

    bool OnGetNodeList(std::shared_ptr<HttpRequest> req);

  private:
    // 服务表
    map<int, ServerInfo> node_map_;                // ikey -> node id, value -> ServerInfo
    std::map<int, std::set<int>> nodes_subscribe_; // key -> node type, value -> node ids;

    INetModule *m_net_;
    ILogModule *m_log_;
    INodeModule *m_node_;
    IHttpServerModule *m_http_server_;
    time_t last_report_time_;
};

} // namespace master::logic