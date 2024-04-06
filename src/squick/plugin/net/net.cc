

#include <atomic>
#include <string.h>

#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include <event2/event.h>

#include <squick/core/base.h>
#include <squick/core/exception.h>

#include "net.h"

/*
Any one who want to upgrade the networking library(libEvent), please change the size of evbuffer showed below:
*MODIFY--libevent/buffer.c
#define EVBUFFER_MAX_READ	4096
TO
#define EVBUFFER_MAX_READ	65536
*/

// 1048576 = 1024 * 1024
#define SQUICK_BUFFER_MAX_READ 1048576

void Net::event_fatal_cb(int err) {
    // dout << "event_fatal_cb:  " << err << std::endl;
}
void Net::conn_writecb(struct bufferevent *bev, void *user_data) {

    //  struct evbuffer *output = bufferevent_get_output(bev);
}

void Net::conn_eventcb(struct bufferevent *bev, short events, void *user_data) {
    NetObject *pObject = (NetObject *)user_data;
    Net *pNet = (Net *)pObject->GetNet();
    //dout << "网络事件 : Thread ID = " << std::this_thread::get_id() << " FD = " << pObject->GetRealFD() << " Event ID =" << events
    //           << std::endl;
    if (events & BEV_EVENT_CONNECTED) {
        // must to set it's state before the "EventCB" functional be called[maybe user will send msg in the callback function]
        pNet->mbWorking = true;
    } else {
        if (!pNet->mbServer) {
            pNet->mbWorking = false;
        }
    }

    if (pNet->mEventCB) {
        pNet->mEventCB(pObject->GetRealFD(), SQUICK_NET_EVENT(events), pNet); // 处理上层绑定的回调
    }

    if (events & BEV_EVENT_CONNECTED) {
        struct evbuffer *input = bufferevent_get_input(bev);
        struct evbuffer *output = bufferevent_get_output(bev);
        if (pNet->expand_buffer_size_ > 0) {
            evbuffer_expand(input, pNet->expand_buffer_size_);
            evbuffer_expand(output, pNet->expand_buffer_size_);
        }
        // printf("%d Connection successed\n", pObject->GetFd());/*XXX win32*/
    } else {
        pNet->CloseNetObject(pObject->GetRealFD());
    }
}

void Net::listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data) {

    Net *pNet = (Net *)user_data;
    bool bClose = pNet->CloseNetObject(fd);
    if (bClose) {
        return;
    }

    if (pNet->mmObject.size() >= pNet->mnMaxConnect) {

        return;
    }

    struct event_base *mxBase = pNet->mxBase;

    struct bufferevent *bev = bufferevent_socket_new(mxBase, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Error constructing bufferevent!");
        return;
    }

    struct sockaddr_in *pSin = (sockaddr_in *)sa;

    NetObject *pObject = new NetObject(pNet, fd, *pSin, bev);
    pObject->GetNet()->AddNetObject(fd, pObject);

#if PLATFORM != PLATFORM_WIN
    int optval = 1;
    int result = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
    // setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));
    if (result < 0) {
        std::cout << "setsockopt TCP_NODELAY ERROR !!!" << std::endl;
    }

    int nRecvBufLen = SQUICK_BUFFER_MAX_READ;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char *)&nRecvBufLen, sizeof(int));
#endif

    bufferevent_setcb(bev, conn_readcb, conn_writecb, conn_eventcb, (void *)pObject);

    bufferevent_enable(bev, EV_READ | EV_WRITE | EV_CLOSED | EV_FINALIZE | EV_TIMEOUT | EV_PERSIST | EV_SIGNAL);

    event_set_fatal_callback(event_fatal_cb);

    conn_eventcb(bev, BEV_EVENT_CONNECTED, (void *)pObject);

    bufferevent_set_max_single_read(bev, SQUICK_BUFFER_MAX_READ);
    bufferevent_set_max_single_write(bev, SQUICK_BUFFER_MAX_READ);
}

void Net::conn_readcb(struct bufferevent *bev, void *user_data) {
    NetObject *pObject = (NetObject *)user_data;
    if (!pObject) {
        return;
    }

    Net *pNet = (Net *)pObject->GetNet();
    if (!pNet) {
        return;
    }

    if (pObject->NeedRemove()) {
        return;
    }

    struct evbuffer *input = bufferevent_get_input(bev);
    if (!input) {
        return;
    }

    size_t len = evbuffer_get_length(input);

    unsigned char *pData = evbuffer_pullup(input, len);
    pObject->AddBuff((const char *)pData, len);
    evbuffer_drain(input, len);

    if (pNet->mbTCPStream) {
        int len = pObject->GetBuffLen();
        if (len > 0) {
            if (pNet->mRecvCB) {
                pNet->mRecvCB(pObject->GetRealFD(), -1, pObject->GetBuff(), len);
                pNet->mnReceiveMsgTotal++;
            }

            pObject->RemoveBuff(0, len);
        }
    } else {
        while (1) {
            if (!pNet->Dismantle(pObject)) {
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////

bool Net::Update() {
    UpdateClose();

    if (mxBase) {
        event_base_loop(mxBase, EVLOOP_ONCE | EVLOOP_NONBLOCK);
    }

    return true;
}

void Net::Connect(const char *ip, const unsigned short nPort, const uint32_t expand_buffer_size) {
    mstrIP = ip;
    mnPort = nPort;
    expand_buffer_size_ = expand_buffer_size;

    StartClientNet();
}

int Net::Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount, const uint32_t expand_buffer_size) {
    mnMaxConnect = nMaxClient;
    mnPort = nPort;
    mnCpuCount = nCpuCount;
    expand_buffer_size_ = expand_buffer_size;
    return StartServerNet();
}

bool Net::Final() {

    CloseSocketAll();

    if (listener) {
        evconnlistener_free(listener);
        listener = NULL;
    }

    if (mxBase) {
        event_base_free(mxBase);
        mxBase = NULL;
    }

    return true;
}

bool Net::SendMsgToAllClient(const char *msg, const size_t len) {
    if (len <= 0) {
        return false;
    }

    if (!mbWorking) {
        return false;
    }

    auto it = mmObject.begin();
    for (; it != mmObject.end(); ++it) {
        NetObject *pNetObject = (NetObject *)it->second;
        if (pNetObject && !pNetObject->NeedRemove()) {
            bufferevent *bev = (bufferevent *)pNetObject->GetUserData();
            if (NULL != bev) {
                bufferevent_write(bev, msg, len);

                mnSendMsgTotal++;
            }
        }
    }

    return true;
}

bool Net::SendMsg(const char *msg, const size_t len, const socket_t sock) {
    if (len <= 0) {
        return false;
    }

    if (!mbWorking) {
        return false;
    }

    auto it = mmObject.find(sock);
    if (it != mmObject.end()) {
        NetObject *pNetObject = (NetObject *)it->second;
        if (pNetObject) {
            bufferevent *bev = (bufferevent *)pNetObject->GetUserData();
            if (NULL != bev) {
                bufferevent_write(bev, msg, len);
                mnSendMsgTotal++;
                return true;
            }
        }
    }

    return false;
}

bool Net::SendMsg(const char *msg, const size_t len, const std::list<socket_t> &fdList) {
    auto it = fdList.begin();
    for (; it != fdList.end(); ++it) {
        SendMsg(msg, len, *it);
    }

    return true;
}

bool Net::CloseNetObject(const socket_t sock) {
    auto it = mmObject.find(sock);
    if (it != mmObject.end()) {
        NetObject *pObject = it->second;

        pObject->SetNeedRemove(true);
        mvRemoveObject.push_back(sock);

        return true;
    }

    return false;
}

// 拆包
bool Net::Dismantle(NetObject *pObject) {
    bool bNeedDismantle = false;
    
    int len = pObject->GetBuffLen();
    if (len >= IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH) {
        rpcHead xHead;
        int nMsgBodyLength = DeCode(pObject->GetBuff(), len, xHead); // 解析头部
        if (nMsgBodyLength >= 0 && xHead.GetMsgID() > 0) {
            if (mRecvCB) {
#if PLATFORM != PLATFORM_WIN
                try {
#endif
                    mRecvCB(pObject->GetRealFD(), xHead.GetMsgID(), pObject->GetBuff() + IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH, nMsgBodyLength);

#if PLATFORM != PLATFORM_WIN
                } catch (const std::exception &e) {
                    Exception::StackTrace(xHead.GetMsgID());
                } catch (...) {
                    Exception::StackTrace(xHead.GetMsgID());
                }
#endif
                mnReceiveMsgTotal++;
            }

            pObject->RemoveBuff(0, nMsgBodyLength + IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH);
            bNeedDismantle = true;
        }else {
            // pObject->IncreaseError();
            bNeedDismantle = false;
        }
    }

    return bNeedDismantle;
}

bool Net::AddNetObject(const socket_t sock, NetObject *pObject) {
    // lock
    return mmObject.insert(std::map<socket_t, NetObject *>::value_type(sock, pObject)).second;
}

int Net::StartClientNet() {
    std::string ip = mstrIP;
    int nPort = mnPort;

    struct sockaddr_in addr;
    struct bufferevent *bev = NULL;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(nPort);

    if (evutil_inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        printf("inet_pton");
        return -1;
    }

    mxBase = event_base_new();
    if (mxBase == NULL) {
        printf("event_base_new ");
        return -1;
    }

    bev = bufferevent_socket_new(mxBase, -1, BEV_OPT_CLOSE_ON_FREE);
    if (bev == NULL) {
        printf("bufferevent_socket_new ");
        return -1;
    }

    int bRet = bufferevent_socket_connect(bev, (struct sockaddr *)&addr, sizeof(addr));
    if (0 != bRet) {
        // int nError = GetLastError();
        printf("bufferevent_socket_connect error");
        return -1;
    }

    socket_t sockfd = bufferevent_getfd(bev);
    NetObject *pObject = new NetObject(this, sockfd, addr, bev);
    if (!AddNetObject(0, pObject)) {
        assert(0);
        return -1;
    }

    mbServer = false;

#if PLATFORM != PLATFORM_WIN
    int optval = 1;
    int result = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
    // setsockopt(fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof(optval));
    if (result < 0) {
        std::cout << "setsockopt TCP_NODELAY ERROR !!!" << std::endl;
    }
    int nRecvBufLen = SQUICK_BUFFER_MAX_READ;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char *)&nRecvBufLen, sizeof(int));
#endif

    bufferevent_setcb(bev, conn_readcb, conn_writecb, conn_eventcb, (void *)pObject);
    bufferevent_enable(bev, EV_READ | EV_WRITE | EV_CLOSED | EV_TIMEOUT | EV_PERSIST);

    event_set_log_callback(&Net::log_cb);

    bufferevent_set_max_single_read(bev, SQUICK_BUFFER_MAX_READ);
    bufferevent_set_max_single_write(bev, SQUICK_BUFFER_MAX_READ);

    int nSizeRead = (int)bufferevent_get_max_to_read(bev);
    int nSizeWrite = (int)bufferevent_get_max_to_write(bev);

    //std::cout << "want to connect " << mstrIP << ":" << nPort << " SizeRead: " << nSizeRead << std::endl;
    //std::cout << "SizeWrite: " << nSizeWrite << std::endl;

    return sockfd;
}

int Net::StartServerNet() {
    int nCpuCount = mnCpuCount;
    int nPort = mnPort;

    struct sockaddr_in sin;
    //////////////////////////////////////////////////////////////////////////

    struct event_config *cfg = event_config_new();

    // event_config_avoid_method(cfg, "epoll");
    if (event_config_set_flag(cfg, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST) < 0) {

        return -1;
    }

    mxBase = event_base_new_with_config(cfg); // event_base_new()

    event_config_free(cfg);

    //////////////////////////////////////////////////////////////////////////

    if (!mxBase) {
        fprintf(stderr, "Could not initialize libevent!\n");
        Final();

        return -1;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(nPort);

    listener = evconnlistener_new_bind(mxBase, listener_cb, (void *)this, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr *)&sin, sizeof(sin));

    if (!listener) {
        fprintf(stderr, "Could not create a listener! listen port: %d\n", nPort);
        perror("Error: ");
        Final();
        return -1;
    }

    mbServer = true;

    event_set_log_callback(&Net::log_cb);

    return mnMaxConnect;
}

bool Net::CloseSocketAll() {
    auto it = mmObject.begin();
    for (; it != mmObject.end(); ++it) {
        socket_t nFD = it->first;
        mvRemoveObject.push_back(nFD);
    }

    UpdateClose();

    mmObject.clear();

    return true;
}

NetObject *Net::GetNetObject(const socket_t sock) {
    auto it = mmObject.find(sock);
    if (it != mmObject.end()) {
        return it->second;
    }

    return NULL;
}

void Net::CloseObject(const socket_t sock) {
    auto it = mmObject.find(sock);
    if (it != mmObject.end()) {
        NetObject *pObject = it->second;

        struct bufferevent *bev = (bufferevent *)pObject->GetUserData();

        bufferevent_free(bev);

        mmObject.erase(it);

        delete pObject;
        pObject = NULL;
    }
}

void Net::UpdateClose() {
    for (int i = 0; i < mvRemoveObject.size(); ++i) {
        socket_t nSocketIndex = mvRemoveObject[i];
        CloseObject(nSocketIndex);
    }

    mvRemoveObject.clear();
}

void Net::log_cb(int severity, const char *msg) {
    //LOG(FATAL) << "severity:" << severity << " " << msg;
}

bool Net::IsServer() { return mbServer; }

bool Net::Log(int severity, const char *msg) {
    log_cb(severity, msg);
    return true;
}

// Rpc send
bool Net::SendMsgWithOutHead(const int16_t msg_id, const char *msg, const size_t len, const socket_t sock /*= 0*/) {
    std::string strOutData;
    int nAllLen = EnCode(msg_id, msg, len, strOutData);
    if (nAllLen == len + IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH) {
        return SendMsg(strOutData.c_str(), strOutData.length(), sock);
    }

    return false;
}

bool Net::SendMsgWithOutHead(const int16_t msg_id, const char *msg, const size_t len, const std::list<socket_t> &fdList) {
    std::string strOutData;
    int nAllLen = EnCode(msg_id, msg, len, strOutData);
    if (nAllLen == len + IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH) {
        return SendMsg(strOutData.c_str(), strOutData.length(), fdList);
    }

    return false;
}

bool Net::SendMsgToAllClientWithOutHead(const int16_t msg_id, const char *msg, const size_t len) {
    std::string strOutData;
    int nAllLen = EnCode(msg_id, msg, len, strOutData);
    if (nAllLen == len + IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH) {
        return SendMsgToAllClient(strOutData.c_str(), (uint32_t)strOutData.length());
    }

    return false;
}

int Net::EnCode(const uint16_t umsg_id, const char *strData, const uint32_t unDataLen, std::string &strOutData) {
    rpcHead xHead;
    xHead.SetMsgID(umsg_id);
    xHead.SetBodyLength(unDataLen);

    char szHead[IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH] = {0};
    xHead.EnCode(szHead);

    strOutData.clear();
    strOutData.append(szHead, IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH);
    strOutData.append(strData, unDataLen);

    return xHead.GetBodyLength() + IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH;
}

int Net::DeCode(const char *strData, const uint32_t unAllLen, rpcHead &xHead) {

    if (unAllLen < IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH) {

        return -1;
    }

    if (IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH != xHead.DeCode(strData)) {

        return -2;
    }

    if (xHead.GetBodyLength() > (unAllLen - IMsgHead::SQUICK_Head::SQUICK_HEAD_LENGTH)) {

        return -3;
    }

    return xHead.GetBodyLength();
}
