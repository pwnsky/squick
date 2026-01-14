#pragma once

#include <iosfwd>
#include <iostream>

#include "coroutine.h"
#include "i_net.h"
#include <core/base.h>
#include <core/i_module.h>
#include <core/i_plugin_manager.h>
#include <core/queue.h>
#include <core/vector4.h>
#include <plugin/core/log/i_log_module.h>
#include <struct/struct.h>

#define NET_COROTINE_MAX_SURVIVAL_TIME 10

struct ServerInfo {
    ServerInfo() {
        fd = 0;
        info = std::shared_ptr<rpc::Node>(new rpc::Node());
    }
    ~ServerInfo() {
        fd = 0;
        info = NULL;
    }

    enum class Status {
        Unknowing,
        Connecting,
        Connected,
        Disconnected,
    };
    Status status = ServerInfo::Status::Unknowing;
    socket_t fd;
    std::shared_ptr<rpc::Node> info;
    vector<int> listen_types;
};

class INetModule : public IModule {
  public:
    static Vector2 ProtobufToStruct(rpc::Vector2 value) {
        Vector2 vector;
        vector.SetX(value.x());
        vector.SetY(value.y());
        return vector;
    }

    static Vector3 ProtobufToStruct(rpc::Vector3 value) {
        Vector3 vector;
        vector.SetX(value.x());
        vector.SetY(value.y());
        vector.SetZ(value.z());
        return vector;
    }

    static Vector4 ProtobufToStruct(rpc::Vector4 value) {
        Vector4 vector;
        vector.SetX(value.x());
        vector.SetY(value.y());
        vector.SetZ(value.z());
        vector.SetW(value.w());
        return vector;
    }

    static rpc::Vector2 StructToProtobuf(Vector2 value) {
        rpc::Vector2 vector;
        vector.set_x(value.X());
        vector.set_y(value.Y());
        return vector;
    }

    static rpc::Vector3 StructToProtobuf(Vector3 value) {
        rpc::Vector3 vector;
        vector.set_x(value.X());
        vector.set_y(value.Y());
        vector.set_z(value.Z());
        return vector;
    }

    static rpc::Vector4 StructToProtobuf(Vector4 value) {
        rpc::Vector4 vector;
        vector.set_x(value.X());
        vector.set_y(value.Y());
        vector.set_z(value.Z());
        vector.set_w(value.W());
        return vector;
    }

    template <typename BaseType>
    bool AddReceiveCallBack(const uint32_t msg_id, BaseType *pBase, void (BaseType::*handleReceiver)(const socket_t, const uint32_t, const char *, const uint32_t)) {
        NET_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

        return AddReceiveCallBack(msg_id, functorPtr);
    }

    template <typename BaseType>
    bool AddReceiveCallBack(BaseType *pBase, void (BaseType::*handleReceiver)(const socket_t, const uint32_t, const char *, const uint32_t)) {
        NET_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

        return AddReceiveCallBack(functorPtr);
    }

    template <typename BaseType>
    bool AddReceiveCallBack(const uint32_t msg_id, BaseType *pBase,
                            Coroutine<bool> (BaseType::*handleReceiver)(const socket_t, const uint32_t, const char *, const uint32_t)) {
        NET_CORO_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_CORO_RECEIVE_FUNCTOR_PTR functorPtr(new NET_CORO_RECEIVE_FUNCTOR(functor));

        return AddReceiveCallBack(msg_id, functorPtr);
    }

    template <typename BaseType> bool AddEventCallBack(BaseType *pBase, void (BaseType::*handler)(const socket_t, const SQUICK_NET_EVENT, INet *)) {
        NET_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        NET_EVENT_FUNCTOR_PTR functorPtr(new NET_EVENT_FUNCTOR(functor));

        return AddEventCallBack(functorPtr);
    }

    static bool ReceivePB(const uint32_t msg_id, const char *msg, const uint32_t len, std::string &msgData, uint64_t &uid) {
        rpc::MsgBase xMsg;
        if (!xMsg.ParseFromArray(msg, len)) {
            ostringstream str;
#ifdef DEBUG
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
            std::cout << "--------------------" << szData << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif // DEBUG

            return false;
        }

        msgData.assign(xMsg.msg_data().data(), xMsg.msg_data().length());

        uid = xMsg.uid();

        return true;
    }

    static bool ReceivePB(const uint32_t msg_id, const std::string &strMsgData, google::protobuf::Message &xData, uint64_t &uid) {
        return ReceivePB(msg_id, strMsgData.c_str(), (uint32_t)strMsgData.length(), xData, uid);
    }

    static bool ReceivePB(const uint32_t msg_id, const char *msg, const uint32_t len, google::protobuf::Message &xData, uint64_t &uid) {
        rpc::MsgBase xMsg;
        if (!xMsg.ParseFromArray(msg, len)) {

#ifdef DEBUG
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
            std::cout << "--------------------" << szData << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif // DEBUG

            return false;
        }

        if (!xData.ParseFromString(xMsg.msg_data())) {

#ifdef DEBUG
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from MsgData to ProtocolData, MessageID: %d\n", msg_id);
            std::cout << "--------------------" << szData << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif // DEBUG

            return false;
        }
        uid = xMsg.uid();
        return true;
    }

    /////////////////
    // as client
    virtual void Connect(const char *ip, const unsigned short nPort, const uint32_t expand_buffer_size) = 0;

    // as server
    virtual int Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount, const uint32_t expand_buffer_size) = 0;

    virtual void RemoveReceiveCallBack(const uint32_t msg_id) = 0;

    virtual bool AddReceiveCallBack(const uint32_t msg_id, const NET_RECEIVE_FUNCTOR_PTR &cb) = 0;
    virtual bool AddReceiveCallBack(const uint32_t msg_id, const NET_CORO_RECEIVE_FUNCTOR_PTR &cb) = 0;

    virtual bool AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb) = 0;

    virtual bool AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual bool Update() = 0;

    virtual bool SendMsg(const uint32_t msg_id, const std::string &msg, const socket_t sock) = 0;
    virtual bool SendMsgToAllClient(const uint32_t msg_id, const std::string &msg) = 0;
    virtual bool SendPBToNode(const uint32_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const uint64_t uid = 0,
                              reqid_t req_id = 0) = 0;
    virtual bool SendToNode(const uint32_t msg_id, const std::string &xData, const socket_t sock, const uint64_t uid = 0, reqid_t req_id = 0) = 0;
    virtual bool SendPBToAllNodeClient(const uint32_t msg_id, const google::protobuf::Message &xData) = 0;

    virtual INet *GetNet() = 0;
};
