#pragma once

#include "net.h"
#include <iosfwd>
#include <iostream>
#include <squick/core/performance.h>
#include <squick/core/queue.h>
#include <struct/struct.h>

#include "i_net_module.h"
#include <squick/core/exception.h>
#include <squick/core/base.h>
#include <squick/plugin/log/export.h>

class NetModule : public INetModule {
  public:
    NetModule(IPluginManager *p);

    virtual ~NetModule();

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();

    // as client
    virtual void Connect(const char *ip, const unsigned short nPort, const uint32_t expand_buffer_size) override;

    // as server
    virtual int Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount, const uint32_t expand_buffer_size) override;

    virtual void RemoveReceiveCallBack(const int msg_id);
    virtual bool AddReceiveCallBack(const int msg_id, const NET_RECEIVE_FUNCTOR_PTR &cb);
    virtual bool AddReceiveCallBack(const int msg_id, const NET_CORO_RECEIVE_FUNCTOR_PTR& cb);
    virtual bool AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb);
    virtual bool AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb);
    virtual bool SendMsgWithOutHead(const int msg_id, const std::string &msg, const socket_t sock);
    virtual bool SendMsgToAllClientWithOutHead(const int msg_id, const std::string &msg);

    virtual bool SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock);
    virtual bool SendMsgPB(const uint16_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const uint64_t uid, reqid_t req_id = 0);
    virtual bool SendMsg(const uint16_t msg_id, const std::string &xData, const socket_t sock, const uint64_t uid, reqid_t req_id = 0);

    virtual bool SendMsgPBToAllClient(const uint16_t msg_id, const google::protobuf::Message &xData);
    virtual INet *GetNet();

  protected:
    void OnReceiveNetPack(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnSocketNetEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet);
    int FixCoroutines(time_t now_time);

  private:
    unsigned int mnBufferSize;
    INet *m_pNet;
    INT64 nLastTime;
    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>> mxReceiveCallBack;
    std::list<NET_EVENT_FUNCTOR_PTR> mxEventCallBackList;
    std::list<NET_RECEIVE_FUNCTOR_PTR> mxCallBackList;
    std::map<int, std::list< NET_CORO_RECEIVE_FUNCTOR_PTR>> coro_funcs_;
    list<Coroutine<bool>> coroutines_;
    ILogModule *m_log_;
    time_t last_check_coroutines_time_ = 0;
};
