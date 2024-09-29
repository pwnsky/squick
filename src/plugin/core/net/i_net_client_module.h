#pragma once

#include "coroutine.h"
#include "i_net_module.h"
#include <iostream>
#include <core/consistent_hash.h>
#include <core/i_module.h>

enum ConnectDataState {
    DISCONNECT,
    CONNECTING,
    NORMAL,
    RECONNECT,
};

struct ConnectData {
    ConnectData() {
        id = 0;
        port = 0;
        name = "";
        ip = "";
        type = 0;
        state = ConnectDataState::DISCONNECT;
        last_time = 0;
        buffer_size = 0;
    }

    int id;
    int type;
    std::string ip;
    int port;
    std::string name;
    ConnectDataState state;
    INT64 last_time;
    uint32_t buffer_size;
    std::shared_ptr<INetModule> net_module;
};

struct NetClientResponseData {
    bool error;
    reqid_t req_id;
    int req_msg_id;
    int ack_msg_id;
    socket_t sock;
    const char *data;
    size_t length;
};

class INetClientModule : public IModule {
  public:
    enum EConstDefine {
        EConstDefine_DefaultWeith = 500,
    };

    template <typename BaseType>
    bool AddReceiveCallBack(const int eType, const uint16_t msg_id, BaseType *pBase,
                            void (BaseType::*handleReceiver)(const socket_t, const int, const char *, const uint32_t)) {
        NET_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

        AddReceiveCallBack(eType, msg_id, functorPtr);

        return true;
    }

    template <typename BaseType>
    int AddReceiveCallBack(const int eType, BaseType *pBase, void (BaseType::*handleReceiver)(const socket_t, const int, const char *, const uint32_t)) {
        NET_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

        AddReceiveCallBack(eType, functorPtr);

        return true;
    }

    template <typename BaseType>
    bool AddEventCallBack(const int eType, BaseType *pBase, void (BaseType::*handler)(const socket_t, const SQUICK_NET_EVENT, INet *)) {
        NET_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        NET_EVENT_FUNCTOR_PTR functorPtr(new NET_EVENT_FUNCTOR(functor));

        AddEventCallBack(eType, functorPtr);

        return true;
    }

    virtual int AddReceiveCallBack(const int eType, NET_RECEIVE_FUNCTOR_PTR functorPtr) = 0;
    virtual int AddReceiveCallBack(const int eType, const uint16_t msg_id, NET_RECEIVE_FUNCTOR_PTR functorPtr) = 0;
    virtual int AddEventCallBack(const int eType, NET_EVENT_FUNCTOR_PTR functorPtr) = 0;

    virtual void RemoveReceiveCallBack(const int eType, const uint16_t msg_id) = 0;
    ////////////////////////////////////////////////////////////////////////////////

    virtual void AddNode(const ConnectData &xInfo) = 0;

    ////////////////////////////////////////////////////////////////////////////////
    virtual bool IsConnected(const int node_id) = 0;
    virtual bool SendByID(const int serverID, const uint16_t msg_id, const std::string &strData, const uint64_t uid = 0, reqid_t req_id = 0) = 0;
    virtual bool SendPBByID(const int serverID, const uint16_t msg_id, const google::protobuf::Message &xData, const uint64_t uid = 0, reqid_t req_id = 0) = 0;
    virtual void SendToAllNode(const uint16_t msg_id, const std::string &strData, const uint64_t uid = 0) = 0;
    virtual void SendToAllNodeByType(const int eType, const uint16_t msg_id, const std::string &strData, const uint64_t uid = 0) = 0;
    virtual void SendPBToAllNode(const uint16_t msg_id, const google::protobuf::Message &xData, const uint64_t uid = 0) = 0;
    virtual void SendPBToAllNodeByType(const int eType, const uint16_t msg_id, const google::protobuf::Message &xData, const uint64_t uid = 0) = 0;
    ////////////////////////////////////////////////////////////////////////////////
    // coroutine
    virtual Awaitable<NetClientResponseData> Request(const int serverID, const uint16_t msg_id, const std::string &data, int ack_msg_id,
                                                     const uint64_t uid = 0) = 0;
    virtual Awaitable<NetClientResponseData> RequestPB(const int node_id, const uint16_t msg_id, const google::protobuf::Message &pb, int ack_msg_id,
                                                       const uint64_t uid = 0) = 0;

    virtual MapEx<int, ConnectData> &GetServerList() = 0;
    virtual std::shared_ptr<ConnectData> GetServerNetInfo(const int serverID) = 0;
    virtual std::shared_ptr<ConnectData> GetServerNetInfo(const INet *pNet) = 0;

    virtual int GetConnections() = 0;
    virtual int GetRandomNodeID(int node_type) = 0;
};
