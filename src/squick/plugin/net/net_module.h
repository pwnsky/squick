#pragma once

#include "net.h"
#include <iosfwd>
#include <iostream>
#include <squick/core/performance.h>
#include <squick/core/queue.h>
#include <squick/struct/struct.h>

#include "i_net_module.h"
#include <squick/core/exception.h>
#include <squick/core/i_plugin_manager.h>
#include <squick/plugin/log/i_log_module.h>

class NetModule : public INetModule {
  public:
    NetModule(IPluginManager *p);

    virtual ~NetModule();

    virtual bool Start();
    virtual bool AfterStart();

    // as client
    virtual void Startialization(const char *ip, const unsigned short nPort);

    // as server
    virtual int Startialization(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount = 4);

    virtual unsigned int ExpandBufferSize(const unsigned int size = 1024 * 1024 * 20) override;

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

    void KeepAlive();

  private:
    unsigned int mnBufferSize;
    INet *m_pNet;
    INT64 nLastTime;
    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>> mxReceiveCallBack;
    std::list<NET_EVENT_FUNCTOR_PTR> mxEventCallBackList;
    std::list<NET_RECEIVE_FUNCTOR_PTR> mxCallBackList;

    ILogModule *m_log_;
};
