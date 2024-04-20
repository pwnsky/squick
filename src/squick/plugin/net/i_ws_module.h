#pragma once

#include "i_net.h"
#include <iosfwd>
#include <iostream>
#include <squick/core/guid.h>
#include <squick/core/i_module.h>
#include <squick/core/i_plugin_manager.h>
#include <squick/core/queue.h>
#include <squick/plugin/log/i_log_module.h>
#include <struct/struct.h>

class IWSModule : public IModule {
  public:
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

    // as client
    virtual void Connect(const char *ip, const unsigned short nPort, const uint32_t expand_buffer_size) = 0;

    // as server
    virtual int Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount, const uint32_t expand_buffer_size) = 0;

    virtual void RemoveReceiveCallBack(const int msg_id) = 0;
    virtual bool AddReceiveCallBack(const int msg_id, const NET_RECEIVE_FUNCTOR_PTR &cb) = 0;
    virtual bool AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb) = 0;
    virtual bool AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb) = 0;

    virtual bool Update() = 0;

    virtual bool SendData(const std::string &msg, const socket_t sock, const bool text = true) = 0;
    virtual bool SendDataToAllClient(const std::string &msg, const bool text = true) = 0;

    virtual bool SendMsg(const int16_t msg_id, const char *msg, const size_t len, const socket_t sock) = 0;
    virtual bool SendPBMsg(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock) = 0;

    virtual INet *GetNet() = 0;
};
