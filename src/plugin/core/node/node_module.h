#pragma once

#include "i_node_module.h"
#include <core/map.h>
#include <plugin/core/log/export.h>
#include <plugin/core/net/export.h>
#include <struct/struct.h>

#define NODE_REPORT_TIME 10
class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager *p);
    virtual bool Awake() override;
    virtual bool Start() override;
    virtual bool Update() override;

    inline void CalcWorkLoad();
    virtual int CalcConnectionWorkLoad(int connections) override;
    static std::string EnumNodeTypeToString(rpc::NodeType type);
    static rpc::NodeType StringNodeTypeToEnum(const std::string &type);

    virtual ServerInfo &GetNodeInfo() override;

    // Add upper server
    virtual bool AddSubscribeNode(const vector<int> &types) override;
    void InvalidMessage(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);

    // Add upper server
    void OnDynamicServerAdd(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);

  private:
    bool Listen();
    virtual void OnReloadConfig(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    virtual void OnReloadLua(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);

    // Report to upper server
    void UpdateState();

    // 作为服务的监听socket状态事件
    void OnServerSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);
    virtual void OnClientConnected(socket_t sock);
    virtual void OnClientDisconnected(socket_t sock);

    // 作为客户端连接socket事件
    void OnClientSocketEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);
    void OnUpperNodeConnected(INet *pNet);
    void OnNAckNodeRegister(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);

    // Add node ntf
    void OnNNtfNodeAdd(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    bool AddNodes(const google::protobuf::RepeatedPtrField<rpc::Node> &list, bool from_ntf = false);

    // Add node ntf
    void OnNNtfNodeRemove(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    bool RemoveNodes();

    void OnNReqReload(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);

    // Run lua code
    void OnNReqExecuteLua(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);

  public:
    ILogModule *m_log_;
    INetModule *m_net_;
    INetClientModule *m_net_client_;
    ServerInfo node_info_;

  private:
    time_t last_report_time_ = 0;
    time_t last_update_time_ = 0;
    int workload_;

  protected:
};
