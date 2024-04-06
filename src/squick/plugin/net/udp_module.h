#pragma once

#include "net_module.h"
#include <event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include <event2/event_compat.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event2/util.h>
#include <iostream>

class IUDPModule : public INetModule {
  public:
};

class UDPModule : public IUDPModule {
  public:
    UDPModule(IPluginManager *p);

    virtual ~UDPModule();

    virtual bool Start() override;
    virtual bool AfterStart();

    // as client
    virtual void Connect(const char *ip, const unsigned short nPort, const uint32_t expand_buffer_size = 0) override {};

    // as server
    virtual int Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount = 4, const uint32_t expand_buffer_size = 0) override;

    virtual void RemoveReceiveCallBack(const int msg_id);

    virtual bool AddReceiveCallBack(const int msg_id, const NET_RECEIVE_FUNCTOR_PTR &cb);

    virtual bool AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb);

    virtual bool AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb);

    virtual bool Update();

    virtual bool SendMsgWithOutHead(const int msg_id, const std::string &msg, const socket_t sock);
    virtual bool SendMsgToAllClientWithOutHead(const int msg_id, const std::string &msg);

    virtual bool SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock);
    virtual bool SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const Guid id);
    virtual bool SendMsg(const uint16_t msg_id, const std::string &xData, const socket_t sock);
    virtual bool SendMsg(const uint16_t msg_id, const std::string &xData, const socket_t sock, const Guid id);

    virtual bool SendMsgPBToAllClient(const uint16_t msg_id, const google::protobuf::Message &xData);

    virtual bool SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const std::vector<Guid> *pClientIDList);
    virtual bool SendMsgPB(const uint16_t msg_id, const std::string &strData, const socket_t sock, const std::vector<Guid> *pClientIDList);

    virtual INet *GetNet();

  protected:
    void OnReceiveNetPack(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    void OnSocketNetEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);

  private:
    struct event_base *mxBase = nullptr;
    struct event udp_event;
    unsigned int mnBufferSize;
    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>> mxReceiveCallBack;
    std::list<NET_EVENT_FUNCTOR_PTR> mxEventCallBackList;
    std::list<NET_RECEIVE_FUNCTOR_PTR> mxCallBackList;

    ILogModule *m_log_;
};