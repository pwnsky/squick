#pragma once

#include "i_net_module.h"
#include <iostream>
#include <squick/core/consistent_hash.h>
#include <squick/core/i_module.h>

enum ConnectDataState {
    DISCONNECT,
    CONNECTING,
    NORMAL,
    RECONNECT,
};

struct ConnectData {
    ConnectData() {
        nGameID = 0;
        nPort = 0;
        name = "";
        ip = "";
        eServerType = SQUICK_ST_NONE;
        eState = ConnectDataState::DISCONNECT;
        mnLastActionTime = 0;
        nWorkLoad = 0;
    }

    int nGameID;
    SQUICK_SERVER_TYPES eServerType;
    std::string ip;
    int nPort;
    int nWorkLoad;
    std::string name;
    ConnectDataState eState;
    INT64 mnLastActionTime;

    SQUICK_SHARE_PTR<INetModule> mxNetModule;
};

class INetClientModule : public IModule {
  public:
    enum EConstDefine {
        EConstDefine_DefaultWeith = 500,
    };

    template <typename BaseType>
    bool AddReceiveCallBack(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, BaseType *pBase,
                            void (BaseType::*handleReceiver)(const SQUICK_SOCKET, const int, const char *, const uint32_t)) {
        NET_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

        AddReceiveCallBack(eType, msgID, functorPtr);

        return true;
    }

    template <typename BaseType>
    int AddReceiveCallBack(const SQUICK_SERVER_TYPES eType, BaseType *pBase,
                           void (BaseType::*handleReceiver)(const SQUICK_SOCKET, const int, const char *, const uint32_t)) {
        NET_RECEIVE_FUNCTOR functor =
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        NET_RECEIVE_FUNCTOR_PTR functorPtr(new NET_RECEIVE_FUNCTOR(functor));

        AddReceiveCallBack(eType, functorPtr);

        return true;
    }

    template <typename BaseType>
    bool AddEventCallBack(const SQUICK_SERVER_TYPES eType, BaseType *pBase, void (BaseType::*handler)(const SQUICK_SOCKET, const SQUICK_NET_EVENT, INet *)) {
        NET_EVENT_FUNCTOR functor = std::bind(handler, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        NET_EVENT_FUNCTOR_PTR functorPtr(new NET_EVENT_FUNCTOR(functor));

        AddEventCallBack(eType, functorPtr);

        return true;
    }

    virtual int AddReceiveCallBack(const SQUICK_SERVER_TYPES eType, NET_RECEIVE_FUNCTOR_PTR functorPtr) = 0;
    virtual int AddReceiveCallBack(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, NET_RECEIVE_FUNCTOR_PTR functorPtr) = 0;
    virtual int AddEventCallBack(const SQUICK_SERVER_TYPES eType, NET_EVENT_FUNCTOR_PTR functorPtr) = 0;

    virtual void RemoveReceiveCallBack(const SQUICK_SERVER_TYPES eType, const uint16_t msgID) = 0;
    ////////////////////////////////////////////////////////////////////////////////

    virtual void AddServer(const ConnectData &xInfo) = 0;
    virtual unsigned int ExpandBufferSize(const unsigned int size = 1024 * 1024 * 20) = 0;

    ////////////////////////////////////////////////////////////////////////////////
    virtual void SendByServerIDWithOutHead(const int serverID, const uint16_t msgID, const std::string &strData) = 0;

    virtual void SendByServerID(const int serverID, const uint16_t msgID, const std::string &strData) = 0;
    virtual void SendByServerID(const int serverID, const uint16_t msgID, const std::string &strData, const Guid id) = 0;

    virtual void SendToAllServerWithOutHead(const uint16_t msgID, const std::string &strData) = 0;

    virtual void SendToAllServer(const uint16_t msgID, const std::string &strData) = 0;
    virtual void SendToAllServer(const uint16_t msgID, const std::string &strData, const Guid id) = 0;

    virtual void SendToAllServerWithOutHead(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const std::string &strData) = 0;

    virtual void SendToAllServer(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const std::string &strData) = 0;
    virtual void SendToAllServer(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const std::string &strData, const Guid id) = 0;

    virtual void SendToServerByPB(const int serverID, const uint16_t msgID, const google::protobuf::Message &xData) = 0;
    virtual void SendToServerByPB(const int serverID, const uint16_t msgID, const google::protobuf::Message &xData, const Guid id) = 0;

    virtual void SendToAllServerByPB(const uint16_t msgID, const google::protobuf::Message &xData, const Guid id) = 0;
    virtual void SendToAllServerByPB(const SQUICK_SERVER_TYPES eType, const uint16_t msgID, const google::protobuf::Message &xData, const Guid id) = 0;

    ////////////////////////////////////////////////////////////////////////////////

    virtual void SendBySuitWithOutHead(const SQUICK_SERVER_TYPES eType, const std::string &strHashKey, const uint16_t msgID, const std::string &strData) = 0;

    virtual void SendBySuit(const SQUICK_SERVER_TYPES eType, const std::string &strHashKey, const uint16_t msgID, const std::string &strData) = 0;
    virtual void SendBySuit(const SQUICK_SERVER_TYPES eType, const std::string &strHashKey, const uint16_t msgID, const std::string &strData,
                            const Guid id) = 0;

    virtual void SendBySuitWithOutHead(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const std::string &strData) = 0;

    virtual void SendBySuit(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const std::string &strData) = 0;
    virtual void SendBySuit(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const std::string &strData, const Guid id) = 0;

    virtual void SendSuitByPB(const SQUICK_SERVER_TYPES eType, const std::string &strHashKey, const uint16_t msgID, const google::protobuf::Message &xData) = 0;
    virtual void SendSuitByPB(const SQUICK_SERVER_TYPES eType, const std::string &strHashKey, const uint16_t msgID, const google::protobuf::Message &xData,
                              const Guid id) = 0;

    virtual void SendSuitByPB(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const google::protobuf::Message &xData) = 0;
    virtual void SendSuitByPB(const SQUICK_SERVER_TYPES eType, const int nHashKey32, const uint16_t msgID, const google::protobuf::Message &xData,
                              const Guid id) = 0;

    ////////////////////////////////////////////////////////////////////////////////

    virtual MapEx<int, ConnectData> &GetServerList() = 0;
    virtual SQUICK_SHARE_PTR<ConnectData> GetServerNetInfo(const SQUICK_SERVER_TYPES eType) = 0;
    virtual SQUICK_SHARE_PTR<ConnectData> GetServerNetInfo(const int serverID) = 0;
    virtual SQUICK_SHARE_PTR<ConnectData> GetServerNetInfo(const INet *pNet) = 0;
};
