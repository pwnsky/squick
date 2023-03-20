
#ifndef SQUICK_NET_H
#define SQUICK_NET_H

#include "i_net.h"
#include "third_party/concurrentqueue/concurrentqueue.h"
#include <algorithm>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include <event2/event_compat.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event2/util.h>
#include <set>
#include <thread>

#pragma pack(push, 1)

class Net : public INet {
  public:
    Net() {
        mxBase = NULL;
        listener = NULL;

        mstrIP = "";
        mnPort = 0;
        mnCpuCount = 0;
        mbServer = false;
        mbWorking = false;

        mnSendMsgTotal = 0;
        mnReceiveMsgTotal = 0;

        mnBufferSize = 0;
        mbTCPStream = false;
    }

    template <typename BaseType>
    Net(BaseType *pBaseType, void (BaseType::*handleReceive)(const SQUICK_SOCKET, const int, const char *, const uint32_t),
        void (BaseType::*handleEvent)(const SQUICK_SOCKET, const SQUICK_NET_EVENT, INet *), bool tcpStream = false) {
        mxBase = NULL;
        listener = NULL;

        mRecvCB = std::bind(handleReceive, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        mEventCB = std::bind(handleEvent, pBaseType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        mstrIP = "";
        mnPort = 0;
        mnCpuCount = 0;
        mbServer = false;
        mbWorking = false;

        mnSendMsgTotal = 0;
        mnReceiveMsgTotal = 0;

        mnBufferSize = 0;
        mbTCPStream = tcpStream;
    }

    virtual ~Net(){};

  public:
    virtual bool Update() override;

    virtual void Startialization(const char *ip, const unsigned short nPort) override;
    virtual int Startialization(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount = 4) override;
    virtual unsigned int ExpandBufferSize(const unsigned int size) override;

    virtual bool Final() override;

    virtual bool SendMsg(const char *msg, const size_t len, const SQUICK_SOCKET sockIndex) override;

    virtual bool SendMsgWithOutHead(const int16_t msgID, const char *msg, const size_t len, const SQUICK_SOCKET sockIndex) override;

    bool SendMsgToAllClient(const char *msg, const size_t len) override;

    virtual bool SendMsgToAllClientWithOutHead(const int16_t msgID, const char *msg, const size_t len) override;

    virtual bool CloseNetObject(const SQUICK_SOCKET sockIndex) override;
    virtual bool AddNetObject(const SQUICK_SOCKET sockIndex, NetObject *pObject) override;
    virtual NetObject *GetNetObject(const SQUICK_SOCKET sockIndex) override;

    virtual bool IsServer() override;
    virtual bool Log(int severity, const char *msg) override;

  private:
    bool SendMsgWithOutHead(const int16_t msgID, const char *msg, const size_t len, const std::list<SQUICK_SOCKET> &fdList);

    bool SendMsg(const char *msg, const size_t len, const std::list<SQUICK_SOCKET> &fdList);

  private:
    void UpdateClose();
    bool CloseSocketAll();

    bool Dismantle(NetObject *pObject);

    int StartClientNet();
    int StartServerNet();
    void CloseObject(const SQUICK_SOCKET sockIndex);

    static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data);
    static void conn_readcb(struct bufferevent *bev, void *user_data);
    static void conn_writecb(struct bufferevent *bev, void *user_data);
    static void conn_eventcb(struct bufferevent *bev, short events, void *user_data);
    static void log_cb(int severity, const char *msg);
    static void event_fatal_cb(int err);

  protected:
    int DeCode(const char *strData, const uint32_t ulen, SquickStructHead &xHead);
    int EnCode(const uint16_t umsgID, const char *strData, const uint32_t unDataLen, std::string &strOutData);

  private:
    //<fd,object>

    // std::multiset<NetObject*> mLiveBeatMap;

    // Use share pointer replace C-style pointer
    std::map<SQUICK_SOCKET, NetObject *> mmObject;
    std::vector<SQUICK_SOCKET> mvRemoveObject;

    int mnMaxConnect;
    std::string mstrIP;
    int mnPort;
    int mnCpuCount;
    bool mbServer;

    unsigned int mnBufferSize;

    bool mbWorking;
    bool mbTCPStream;

    int64_t mnSendMsgTotal;
    int64_t mnReceiveMsgTotal;

    struct event_base *mxBase;
    struct evconnlistener *listener;
    //////////////////////////////////////////////////////////////////////////

    NET_RECEIVE_FUNCTOR mRecvCB;
    NET_EVENT_FUNCTOR mEventCB;

    // 1: async thread to process net event & msg and main thread to process logic business(decode binary data to message object)
    // 2: pass a functor when startup net module to decode binary data to message object with async thread
    struct NetEvent {
        SQUICK_NET_EVENT event;
        int fd = 0;
        // std::string* data;
        char *data = nullptr;
        int len = 0;

        void *dataObject = nullptr;
    };
    moodycamel::ConcurrentQueue<NetEvent> msgQueue;
    //////////////////////////////////////////////////////////////////////////
};

#pragma pack(pop)

#endif
