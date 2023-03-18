

#ifndef SQUICK_UDP_SERVER_MODULE_H
#define SQUICK_UDP_SERVER_MODULE_H

#include "net_module.h"
#include <event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
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
    virtual void Startialization(const char *ip, const unsigned short nPort){};

    // as server
    virtual int Startialization(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount = 4);

    virtual unsigned int ExpandBufferSize(const unsigned int size = 1024 * 1024 * 20) override;

    virtual void RemoveReceiveCallBack(const int msgID);

    virtual bool AddReceiveCallBack(const int msgID, const NET_RECEIVE_FUNCTOR_PTR &cb);

    virtual bool AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb);

    virtual bool AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb);

    virtual bool Update();

    virtual bool SendMsgWithOutHead(const int msgID, const std::string &msg, const SQUICK_SOCKET sockIndex);
    virtual bool SendMsgToAllClientWithOutHead(const int msgID, const std::string &msg);

    virtual bool SendMsgPB(const uint16_t msgID, const google::protobuf::Message &xData, const SQUICK_SOCKET sockIndex);
    virtual bool SendMsgPB(const uint16_t msgID, const google::protobuf::Message &xData, const SQUICK_SOCKET sockIndex, const Guid id);
    virtual bool SendMsg(const uint16_t msgID, const std::string &xData, const SQUICK_SOCKET sockIndex);
    virtual bool SendMsg(const uint16_t msgID, const std::string &xData, const SQUICK_SOCKET sockIndex, const Guid id);

    virtual bool SendMsgPBToAllClient(const uint16_t msgID, const google::protobuf::Message &xData);

    virtual bool SendMsgPB(const uint16_t msgID, const google::protobuf::Message &xData, const SQUICK_SOCKET sockIndex, const std::vector<Guid> *pClientIDList);
    virtual bool SendMsgPB(const uint16_t msgID, const std::string &strData, const SQUICK_SOCKET sockIndex, const std::vector<Guid> *pClientIDList);

    virtual INet *GetNet();

  protected:
    void OnReceiveNetPack(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

    void OnSocketNetEvent(const SQUICK_SOCKET sockIndex, const SQUICK_NET_EVENT eEvent, INet *pNet);

  private:
    struct event_base *mxBase = nullptr;
    struct event udp_event;
    unsigned int mnBufferSize;
    std::map<int, std::list<NET_RECEIVE_FUNCTOR_PTR>> mxReceiveCallBack;
    std::list<NET_EVENT_FUNCTOR_PTR> mxEventCallBackList;
    std::list<NET_RECEIVE_FUNCTOR_PTR> mxCallBackList;

    ILogModule *m_pLogModule;
};

#endif
