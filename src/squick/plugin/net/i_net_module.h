#pragma once

#include <iosfwd>
#include <iostream>
#include <squick/core/guid.h>

#include "i_net.h"
#include <squick/core/base.h>
#include <squick/core/i_module.h>
#include <squick/core/i_plugin_manager.h>
#include <squick/core/queue.h>
#include <squick/core/vector4.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/struct/struct.h>

////////////////////////////////////////////////////////////////////////////

// only use this macro when u has entered game server
#define CLIENT_MSG_PROCESS(msg_id, msgData, len, msg)                                                                                                          \
    Guid nPlayerID;                                                                                                                                            \
    msg xMsg;                                                                                                                                                  \
    if (!INetModule::ReceivePB(msg_id, msgData, len, xMsg, nPlayerID)) {                                                                                       \
        m_log_->LogError(Guid(), "Parse msg error " + std::to_string(msg_id).append(" in file: ").append(__FILE__), __FUNCTION__, __LINE__);                   \
        return;                                                                                                                                                \
    }                                                                                                                                                          \
                                                                                                                                                               \
    std::shared_ptr<IObject> pObject = m_kernel_->GetObject(nPlayerID);                                                                                        \
    if (NULL == pObject.get()) {                                                                                                                               \
        m_log_->LogError(nPlayerID, "From client object do not exist, msg_id:  " + std::to_string(msg_id).append(" in file: ").append(__FILE__), __FUNCTION__, \
                         __LINE__);                                                                                                                            \
        return;                                                                                                                                                \
    }

#define CLIENT_MSG_PROCESS_NO_OBJECT(msg_id, msgData, len, msg)                                                                                                \
    Guid nPlayerID;                                                                                                                                            \
    msg xMsg;                                                                                                                                                  \
    if (!INetModule::ReceivePB(msg_id, msgData, len, xMsg, nPlayerID)) {                                                                                       \
        m_log_->LogError(nPlayerID, "Parse msg error " + std::to_string(msg_id).append(" in file: ").append(__FILE__), __FUNCTION__, __LINE__);                \
        return;                                                                                                                                                \
    }

#define CLIENT_MSG_PROCESS_NO_LOG(msg_id, msgData, len, msg)                                                                                                   \
    Guid nPlayerID;                                                                                                                                            \
    msg xMsg;                                                                                                                                                  \
    if (!INetModule::ReceivePB(msg_id, msgData, len, xMsg, nPlayerID)) {                                                                                       \
        return 0;                                                                                                                                              \
    }

//////////////////////////////////////////////////////////////////////////
struct ServerData {
    ServerData() {
        pData = std::shared_ptr<rpc::Server>(new rpc::Server());
        nFD = 0;
    }
    ~ServerData() {
        nFD = 0;
        pData = NULL;
    }

    socket_t nFD;
    std::shared_ptr<rpc::Server> pData;
};

class INetModule : public IModule {
  public:
    static Guid ProtobufToStruct(rpc::Ident xID) {
        Guid xIdent;
        xIdent.nHead64 = xID.svrid();
        xIdent.nData64 = xID.index();

        return xIdent;
    }

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

    static rpc::Ident StructToProtobuf(Guid xID) {
        rpc::Ident xIdent;
        xIdent.set_svrid(xID.nHead64);
        xIdent.set_index(xID.nData64);

        return xIdent;
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
    bool AddReceiveCallBack(const int msg_id, BaseType *pBase, void (BaseType::*handleReceiver)(const socket_t, const int, const char *, const uint32_t)) {
        NET_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

        return AddReceiveCallBack(msg_id, functorPtr);
    }

    template <typename BaseType>
    bool AddReceiveCallBack(BaseType *pBase, void (BaseType::*handleReceiver)(const socket_t, const int, const char *, const uint32_t)) {
        NET_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

        return AddReceiveCallBack(functorPtr);
    }

    template <typename BaseType> bool AddEventCallBack(BaseType *pBase, void (BaseType::*handler)(const socket_t, const SQUICK_NET_EVENT, INet *)) {
        NET_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        NET_EVENT_FUNCTOR_PTR functorPtr(new NET_EVENT_FUNCTOR(functor));

        return AddEventCallBack(functorPtr);
    }

    static bool ReceivePB(const int msg_id, const char *msg, const uint32_t len, Guid &nPlayer) {
        rpc::MsgBase xMsg;
        if (!xMsg.ParseFromArray(msg, len)) {
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
#ifdef DEBUG
            std::cout << "--------------------" << szData << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif // DEBUG

            return false;
        }

        nPlayer = ProtobufToStruct(xMsg.player_id());

        return true;
    }

    static bool ReceivePB(const int msg_id, const char *msg, const uint32_t len, std::string &msgData, Guid &nPlayer) {
        rpc::MsgBase xMsg;
        if (!xMsg.ParseFromArray(msg, len)) {
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
#ifdef DEBUG
            std::cout << "--------------------" << szData << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif // DEBUG

            return false;
        }

        msgData.assign(xMsg.msg_data().data(), xMsg.msg_data().length());

        nPlayer = ProtobufToStruct(xMsg.player_id());

        return true;
    }

    static bool ReceivePB(const int msg_id, const std::string &strMsgData, google::protobuf::Message &xData, Guid &nPlayer) {
        return ReceivePB(msg_id, strMsgData.c_str(), (uint32_t)strMsgData.length(), xData, nPlayer);
    }

    static bool ReceivePB(const int msg_id, const char *msg, const uint32_t len, google::protobuf::Message &xData, Guid &nPlayer) {
        rpc::MsgBase xMsg;
        if (!xMsg.ParseFromArray(msg, len)) {
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msg_id);
#ifdef DEBUG
            std::cout << "--------------------" << szData << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif // DEBUG

            return false;
        }

        if (!xData.ParseFromString(xMsg.msg_data())) {
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from MsgData to ProtocolData, MessageID: %d\n", msg_id);
#ifdef DEBUG
            std::cout << "--------------------" << szData << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif // DEBUG

            return false;
        }

        nPlayer = ProtobufToStruct(xMsg.player_id());

        return true;
    }

    /////////////////
    // as client
    virtual void Startialization(const char *ip, const unsigned short nPort) = 0;

    // as server
    virtual int Startialization(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount = 4) = 0;
    virtual unsigned int ExpandBufferSize(const unsigned int size = 1024 * 1024 * 20) = 0;

    virtual void RemoveReceiveCallBack(const int msg_id) = 0;

    virtual bool AddReceiveCallBack(const int msg_id, const NET_RECEIVE_FUNCTOR_PTR &cb) = 0;

    virtual bool AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb) = 0;

    virtual bool AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual bool Update() = 0;

    virtual bool SendMsgWithOutHead(const int msg_id, const std::string &msg, const socket_t sock) = 0;

    virtual bool SendMsgToAllClientWithOutHead(const int msg_id, const std::string &msg) = 0;

    virtual bool SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock) = 0;
    virtual bool SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const Guid nPlayer) = 0;
    virtual bool SendMsg(const uint16_t msg_id, const std::string &xData, const socket_t sock) = 0;
    virtual bool SendMsg(const uint16_t msg_id, const std::string &xData, const socket_t sock, const Guid id) = 0;

    virtual bool SendMsgPBToAllClient(const uint16_t msg_id, const google::protobuf::Message &xData) = 0;

    virtual bool SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const std::vector<Guid> *pClientIDList) = 0;
    virtual bool SendMsgPB(const uint16_t msg_id, const std::string &strData, const socket_t sock, const std::vector<Guid> *pClientIDList) = 0;

    virtual INet *GetNet() = 0;
};
