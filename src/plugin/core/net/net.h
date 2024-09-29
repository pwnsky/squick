#pragma once

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
#include <unordered_map>

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

        mbTCPStream = false;
    }

    template <typename BaseType>
    Net(BaseType *pBaseType, void (BaseType::*handleReceive)(const socket_t, const int, const char *, const uint32_t),
        void (BaseType::*handleEvent)(const socket_t, const SQUICK_NET_EVENT, INet *), bool tcpStream = false) {
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
        mbTCPStream = tcpStream;
    }

    virtual ~Net(){};

  public:
    virtual bool Update() override;

    virtual void Connect(const char *ip, const unsigned short nPort, const uint32_t expand_buffer_size) override;
    virtual int Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount, const uint32_t expand_buffer_size) override;

    virtual bool Final() override;

    virtual bool SendData(const char *data, const size_t len, const socket_t sock) override;
    bool SendDataToAllClient(const char *data, const size_t len) override;

    virtual bool SendMsg(const int16_t msg_id, const char *msg, const size_t len, const socket_t sock) override;
    virtual bool SendMsgToAllClient(const int16_t msg_id, const char *msg, const size_t len) override;

    virtual bool CloseNetObject(const socket_t sock) override;
    virtual bool AddNetObject(const socket_t sock, NetObject *pObject) override;
    virtual NetObject *GetNetObject(const socket_t sock) override;

    virtual bool IsServer() override;
    virtual bool Log(int severity, const char *msg) override;

    virtual int GetConnections() override;

  private:
    bool SendMsg(const int16_t msg_id, const char *msg, const size_t len, const std::list<socket_t> &fdList);
    bool SendData(const char *msg, const size_t len, const std::list<socket_t> &fdList);

  private:
    void UpdateClose();
    bool CloseSocketAll();

    bool Dismantle(NetObject *pObject);

    int StartClientNet();
    int StartServerNet();
    void CloseObject(const socket_t sock);
    bool IsValidIP(const std::string& ip);
    bool IsValidDomain(const std::string& domain);
    std::string GetIpByDomain(std::string domain);

    static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data);
    static void conn_readcb(struct bufferevent *bev, void *user_data);
    static void conn_writecb(struct bufferevent *bev, void *user_data);
    static void conn_eventcb(struct bufferevent *bev, short events, void *user_data);
    static void log_cb(int severity, const char *msg);
    static void event_fatal_cb(int err);

  protected:
    int DeCode(const char *strData, const uint32_t ulen, rpcHead &xHead);
    int EnCode(const uint16_t umsg_id, const char *strData, const uint32_t unDataLen, std::string &strOutData);

  private:
    //<fd,object>

    // std::multiset<NetObject*> mLiveBeatMap;

    // Use share pointer replace C-style pointer
    std::unordered_map<socket_t, NetObject *> mmObject;
    std::vector<socket_t> mvRemoveObject;

    int mnMaxConnect;
    std::string mstrIP;
    int mnPort;
    int mnCpuCount;
    bool mbServer;

    uint32_t expand_buffer_size_ = 0;

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
