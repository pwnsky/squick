#pragma once

#include <iosfwd>
#include <iostream>
#include <squick/core/consistent_hash.h>
#include <squick/core/i_plugin_manager.h>
#include <squick/core/queue.h>
#include <squick/plugin/log/export.h>

#include "i_net_client_module.h"

class NetClientModule : public INetClientModule {
  public:
    NetClientModule(IPluginManager *p);

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool BeforeDestroy();
    virtual bool Destroy();
    virtual bool Update();

    virtual void AddNode(const ConnectData &xInfo);
    virtual int AddReceiveCallBack(const int eType, NET_RECEIVE_FUNCTOR_PTR functorPtr);
    virtual int AddReceiveCallBack(const int eType, const uint16_t msg_id, NET_RECEIVE_FUNCTOR_PTR functorPtr);
    virtual int AddEventCallBack(const int eType, NET_EVENT_FUNCTOR_PTR functorPtr);
    virtual void RemoveReceiveCallBack(const int eType, const uint16_t msg_id);

    virtual bool IsConnected(const int node_id) override;
    virtual bool SendByID(const int serverID, const uint16_t msg_id, const std::string &strData, const uint64_t uid = 0, reqid_t req_id = 0) override;
    virtual bool SendPBByID(const int serverID, const uint16_t msg_id, const google::protobuf::Message &xData, const uint64_t uid = 0,
                            reqid_t req_id = 0) override;
    virtual void SendToAllNode(const uint16_t msg_id, const std::string &strData, const uint64_t uid = 0) override;
    virtual void SendToAllNodeByType(const int eType, const uint16_t msg_id, const std::string &strData, const uint64_t uid = 0) override;
    virtual void SendPBToAllNode(const uint16_t msg_id, const google::protobuf::Message &xData, const uint64_t uid = 0) override;
    virtual void SendPBToAllNodeByType(const int eType, const uint16_t msg_id, const google::protobuf::Message &xData, const uint64_t uid = 0) override;

    // coroutine
    virtual Awaitable<NetClientResponseData> Request(const int node_id, const uint16_t msg_id, const std::string &data, int ack_msg_id,
                                                     const uint64_t uid = 0) override;
    virtual Awaitable<NetClientResponseData> RequestPB(const int node_id, const uint16_t msg_id, const google::protobuf::Message &pb, int ack_msg_id,
                                                       const uint64_t uid = 0) override;

    void CoroutineBinder(Awaitable<NetClientResponseData> *awaitble);
    reqid_t GenerateRequestID();
    void OnAckHandler(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    virtual MapEx<int, ConnectData> &GetServerList();

    virtual std::shared_ptr<ConnectData> GetServerNetInfo(const int serverID);
    virtual std::shared_ptr<ConnectData> GetServerNetInfo(const INet *pNet);

    virtual int GetConnections() override;
    virtual int GetRandomNodeID(int node_type) override;

  protected:
    void StartCallBacks(std::shared_ptr<ConnectData> pServerData);
    void ProcessUpdate();

  private:
    void KeepState(std::shared_ptr<ConnectData> pServerData);
    void OnSocketEvent(const socket_t fd, const SQUICK_NET_EVENT eEvent, INet *pNet);
    int OnConnected(const socket_t fd, INet *pNet);
    int OnDisConnected(const socket_t fd, INet *pNet);
    void ProcessNetConnect();

  private:
    int64_t mnLastActionTime;
    // server_id, server_data
    ConsistentHashMapEx<int, ConnectData> mxServerMap;
    // server_type, server_id, server_data
    MapEx<int, ConsistentHashMapEx<int, ConnectData>> mxServerTypeMap;

    std::list<ConnectData> mxTempNetList;

    struct CallBack {
        // call back
        std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>> mxReceiveCallBack;
        std::list<NET_EVENT_FUNCTOR_PTR> mxEventCallBack;
        std::list<NET_RECEIVE_FUNCTOR_PTR> mxCallBackList;
    };

    MapEx<int, CallBack> mxCallBack;

    ILogModule *m_log_;

    reqid_t last_req_id_ = 0;
    std::map<reqid_t, Awaitable<NetClientResponseData> *> co_awaitbles_;
};
