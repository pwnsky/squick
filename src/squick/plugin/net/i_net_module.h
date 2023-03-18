#pragma once

#include <iosfwd>
#include <iostream>
#include <squick/core/guid.h>

#include "i_net.h"
#include <squick/core/i_module.h>
#include <squick/core/i_plugin_manager.h>
#include <squick/core/queue.h>
#include <squick/core/vector4.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/struct/struct.h>
enum SQUICK_SERVER_TYPES {
    SQUICK_ST_NONE = 0,         // NONE
    SQUICK_ST_REDIS = 1,        //
    SQUICK_ST_MYSQL = 2,        //
    SQUICK_ST_MASTER = 3,       //
    SQUICK_ST_LOGIN = 4,        //
    SQUICK_ST_PROXY = 5,        //
    SQUICK_ST_GAME = 6,         //
    SQUICK_ST_WORLD = 7,        //
    SQUICK_ST_DB_PROXY = 8,           //
    SQUICK_ST_MAX = 9,          //
    SQUICK_ST_GATEWAY = 10,     //
    SQUICK_ST_GAMEPLAY_MANAGER = 11, //
    SQUICK_ST_GAMEPLAY = 12, //
    SQUICK_ST_MICRO = 13,       // 微服务
};

////////////////////////////////////////////////////////////////////////////

// only use this macro when u has entered game server
#define CLIENT_MSG_PROCESS(msgID, msgData, len, msg)                                                                                                           \
    Guid nPlayerID;                                                                                                                                            \
    msg xMsg;                                                                                                                                                  \
    if (!INetModule::ReceivePB(msgID, msgData, len, xMsg, nPlayerID)) {                                                                                        \
        m_pLogModule->LogError(Guid(), "Parse msg error " + std::to_string(msgID).append(" in file: ").append(__FILE__), __FUNCTION__, __LINE__);              \
        return;                                                                                                                                                \
    }                                                                                                                                                          \
                                                                                                                                                               \
    SQUICK_SHARE_PTR<IObject> pObject = m_pKernelModule->GetObject(nPlayerID);                                                                                 \
    if (NULL == pObject.get()) {                                                                                                                               \
        m_pLogModule->LogError(nPlayerID, "From client object do not exist, msg_id:  " + std::to_string(msgID).append(" in file: ").append(__FILE__),          \
                               __FUNCTION__, __LINE__);                                                                                                        \
        return;                                                                                                                                                \
    }

#define CLIENT_MSG_PROCESS_NO_OBJECT(msgID, msgData, len, msg)                                                                                                 \
    Guid nPlayerID;                                                                                                                                            \
    msg xMsg;                                                                                                                                                  \
    if (!INetModule::ReceivePB(msgID, msgData, len, xMsg, nPlayerID)) {                                                                                        \
        m_pLogModule->LogError(nPlayerID, "Parse msg error " + std::to_string(msgID).append(" in file: ").append(__FILE__), __FUNCTION__, __LINE__);           \
        return;                                                                                                                                                \
    }

#define CLIENT_MSG_PROCESS_NO_LOG(msgID, msgData, len, msg)                                                                                                    \
    Guid nPlayerID;                                                                                                                                            \
    msg xMsg;                                                                                                                                                  \
    if (!INetModule::ReceivePB(msgID, msgData, len, xMsg, nPlayerID)) {                                                                                        \
        return 0;                                                                                                                                              \
    }

//////////////////////////////////////////////////////////////////////////
struct ServerData {
    ServerData() {
        pData = SQUICK_SHARE_PTR<SquickStruct::ServerInfoReport>(SQUICK_NEW SquickStruct::ServerInfoReport());
        nFD = 0;
    }
    ~ServerData() {
        nFD = 0;
        pData = NULL;
    }

    SQUICK_SOCKET nFD;
    SQUICK_SHARE_PTR<SquickStruct::ServerInfoReport> pData;
};

class INetModule : public IModule {
  public:
    static Guid ProtobufToStruct(SquickStruct::Ident xID) {
        Guid xIdent;
        xIdent.nHead64 = xID.svrid();
        xIdent.nData64 = xID.index();

        return xIdent;
    }

    static Vector2 ProtobufToStruct(SquickStruct::Vector2 value) {
        Vector2 vector;
        vector.SetX(value.x());
        vector.SetY(value.y());
        return vector;
    }

    static Vector3 ProtobufToStruct(SquickStruct::Vector3 value) {
        Vector3 vector;
        vector.SetX(value.x());
        vector.SetY(value.y());
        vector.SetZ(value.z());
        return vector;
    }

    static Vector4 ProtobufToStruct(SquickStruct::Vector4 value) {
        Vector4 vector;
        vector.SetX(value.x());
        vector.SetY(value.y());
        vector.SetZ(value.z());
        vector.SetW(value.w());
        return vector;
    }

    static SquickStruct::Ident StructToProtobuf(Guid xID) {
        SquickStruct::Ident xIdent;
        xIdent.set_svrid(xID.nHead64);
        xIdent.set_index(xID.nData64);

        return xIdent;
    }

    static SquickStruct::Vector2 StructToProtobuf(Vector2 value) {
        SquickStruct::Vector2 vector;
        vector.set_x(value.X());
        vector.set_y(value.Y());
        return vector;
    }

    static SquickStruct::Vector3 StructToProtobuf(Vector3 value) {
        SquickStruct::Vector3 vector;
        vector.set_x(value.X());
        vector.set_y(value.Y());
        vector.set_z(value.Z());
        return vector;
    }

    static SquickStruct::Vector4 StructToProtobuf(Vector4 value) {
        SquickStruct::Vector4 vector;
        vector.set_x(value.X());
        vector.set_y(value.Y());
        vector.set_z(value.Z());
        vector.set_w(value.W());
        return vector;
    }

    template <typename BaseType>
    bool AddReceiveCallBack(const int msgID, BaseType *pBase, void (BaseType::*handleReceiver)(const SQUICK_SOCKET, const int, const char *, const uint32_t)) {
        NET_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

        return AddReceiveCallBack(msgID, functorPtr);
    }

    template <typename BaseType>
    bool AddReceiveCallBack(BaseType *pBase, void (BaseType::*handleReceiver)(const SQUICK_SOCKET, const int, const char *, const uint32_t)) {
        NET_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

        return AddReceiveCallBack(functorPtr);
    }

    template <typename BaseType> bool AddEventCallBack(BaseType *pBase, void (BaseType::*handler)(const SQUICK_SOCKET, const SQUICK_NET_EVENT, INet *)) {
        NET_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        NET_EVENT_FUNCTOR_PTR functorPtr(new NET_EVENT_FUNCTOR(functor));

        return AddEventCallBack(functorPtr);
    }

    static bool ReceivePB(const int msgID, const char *msg, const uint32_t len, Guid &nPlayer) {
        SquickStruct::MsgBase xMsg;
        if (!xMsg.ParseFromArray(msg, len)) {
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msgID);
#ifdef DEBUG
            std::cout << "--------------------" << szData << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif // DEBUG

            return false;
        }

        nPlayer = ProtobufToStruct(xMsg.player_id());

        return true;
    }

    static bool ReceivePB(const int msgID, const char *msg, const uint32_t len, std::string &msgData, Guid &nPlayer) {
        SquickStruct::MsgBase xMsg;
        if (!xMsg.ParseFromArray(msg, len)) {
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msgID);
#ifdef DEBUG
            std::cout << "--------------------" << szData << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif // DEBUG

            return false;
        }

        msgData.assign(xMsg.msg_data().data(), xMsg.msg_data().length());

        nPlayer = ProtobufToStruct(xMsg.player_id());

        return true;
    }

    static bool ReceivePB(const int msgID, const std::string &strMsgData, google::protobuf::Message &xData, Guid &nPlayer) {
        return ReceivePB(msgID, strMsgData.c_str(), (uint32_t)strMsgData.length(), xData, nPlayer);
    }

    static bool ReceivePB(const int msgID, const char *msg, const uint32_t len, google::protobuf::Message &xData, Guid &nPlayer) {
        SquickStruct::MsgBase xMsg;
        if (!xMsg.ParseFromArray(msg, len)) {
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from Packet to MsgBase, MessageID: %d\n", msgID);
#ifdef DEBUG
            std::cout << "--------------------" << szData << __FUNCTION__ << " " << __LINE__ << std::endl;
#endif // DEBUG

            return false;
        }

        if (!xData.ParseFromString(xMsg.msg_data())) {
            char szData[MAX_PATH] = {0};
            NFSPRINTF(szData, MAX_PATH, "Parse Message Failed from MsgData to ProtocolData, MessageID: %d\n", msgID);
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

    virtual void RemoveReceiveCallBack(const int msgID) = 0;

    virtual bool AddReceiveCallBack(const int msgID, const NET_RECEIVE_FUNCTOR_PTR &cb) = 0;

    virtual bool AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb) = 0;

    virtual bool AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual bool Update() = 0;

    virtual bool SendMsgWithOutHead(const int msgID, const std::string &msg, const SQUICK_SOCKET sockIndex) = 0;

    virtual bool SendMsgToAllClientWithOutHead(const int msgID, const std::string &msg) = 0;

    virtual bool SendMsgPB(const uint16_t msgID, const google::protobuf::Message &xData, const SQUICK_SOCKET sockIndex) = 0;
    virtual bool SendMsgPB(const uint16_t msgID, const google::protobuf::Message &xData, const SQUICK_SOCKET sockIndex, const Guid nPlayer) = 0;
    virtual bool SendMsg(const uint16_t msgID, const std::string &xData, const SQUICK_SOCKET sockIndex) = 0;
    virtual bool SendMsg(const uint16_t msgID, const std::string &xData, const SQUICK_SOCKET sockIndex, const Guid id) = 0;

    virtual bool SendMsgPBToAllClient(const uint16_t msgID, const google::protobuf::Message &xData) = 0;

    virtual bool SendMsgPB(const uint16_t msgID, const google::protobuf::Message &xData, const SQUICK_SOCKET sockIndex,
                           const std::vector<Guid> *pClientIDList) = 0;
    virtual bool SendMsgPB(const uint16_t msgID, const std::string &strData, const SQUICK_SOCKET sockIndex, const std::vector<Guid> *pClientIDList) = 0;

    virtual INet *GetNet() = 0;
};
