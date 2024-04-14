#pragma once

#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <squick/core/guid.h>
#include <vector>
#include "coroutine.h"
#if PLATFORM == PLATFORM_WIN
#include <WinSock2.h>
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_LINUX || PLATFORM == PLATFORM_ANDROID

#if PLATFORM == PLATFORM_APPLE

#include <libkern/OSByteOrder.h>
#include <arpa/inet.h>

#endif

#ifdef _XOPEN_SOURCE_EXTENDED
#include <arpa/inet.h>
#endif

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

enum SQUICK_NET_EVENT {
    SQUICK_NET_EVENT_EOF = 0x10,
    SQUICK_NET_EVENT_ERROR = 0x20,
    SQUICK_NET_EVENT_TIMEOUT = 0x40,
    SQUICK_NET_EVENT_CONNECTED = 0x80,
};

struct IMsgHead {
    enum SQUICK_Head {
        SQUICK_HEAD_LENGTH = 6,
    };

    virtual int EnCode(char *strData) = 0;

    virtual int DeCode(const char *strData) = 0;

    virtual uint16_t GetMsgID() const = 0;

    virtual void SetMsgID(uint16_t msg_id) = 0;

    virtual uint32_t GetBodyLength() const = 0;

    virtual void SetBodyLength(uint32_t length) = 0;

    static uint64_t SQUICK_HTONLL(uint64_t nData) {
#if PLATFORM == PLATFORM_WIN
        return htonll(nData);
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
        return OSSwapHostToBigInt64(nData);
#else
        return htobe64(nData);
#endif
    }

    static uint64_t SQUICK_NTOHLL(uint64_t nData) {
#if PLATFORM == PLATFORM_WIN
        return ntohll(nData);
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
        return OSSwapBigToHostInt64(nData);
#elif PLATFORM == PLATFORM_ANDROID
        return betoh64(nData);
#else
        return be64toh(nData);
#endif
    }

    static uint32_t SQUICK_HTONL(uint32_t nData) {
#if PLATFORM == PLATFORM_WIN
        return htonl(nData);
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
        return OSSwapHostToBigInt32(nData);
#else
        return htobe32(nData);
#endif
    }

    static uint32_t SQUICK_NTOHL(uint32_t nData) {
#if PLATFORM == PLATFORM_WIN
        return ntohl(nData);
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
        return OSSwapBigToHostInt32(nData);
#elif PLATFORM == PLATFORM_ANDROID
        return betoh32(nData);
#else
        return be32toh(nData);
#endif
    }

    static uint16_t SQUICK_HTONS(uint16_t nData) {
#if PLATFORM == PLATFORM_WIN
        return htons(nData);
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
        return OSSwapHostToBigInt16(nData);
#else
        return htobe16(nData);
#endif
    }

    static uint16_t SQUICK_NTOHS(uint16_t nData) {
#if PLATFORM == PLATFORM_WIN
        return ntohs(nData);
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
        return OSSwapBigToHostInt16(nData);
#elif PLATFORM == PLATFORM_ANDROID
        return betoh16(nData);
#else
        return be16toh(nData);
#endif
    }
};

// 协议头部
class rpcHead : public IMsgHead {
  public:
    rpcHead() {
        size_ = 0;
        msg_id_ = 0;
        hashcode_ = 0;
    }

    // Message Head[ MsgID(2) | MsgSize(4) ]
    virtual int EnCode(char *data) {
        uint32_t nOffset = 0;

        uint16_t net_msg_id = SQUICK_HTONS(msg_id_);
        memcpy(data + nOffset, (void *)(&net_msg_id), sizeof(net_msg_id));
        nOffset += sizeof(net_msg_id);

        uint32_t nPackSize = size_ + SQUICK_HEAD_LENGTH;
        uint32_t net_size = SQUICK_HTONL(nPackSize);
        memcpy(data + nOffset, (void *)(&net_size), sizeof(net_size));
        nOffset += sizeof(net_size);

        if (nOffset != SQUICK_HEAD_LENGTH) {
            assert(0);
        }
        return nOffset;
    }

    // Message Head[ MsgID(2) | MsgSize(4) ]
    virtual int DeCode(const char *data) {
        uint32_t nOffset = 0;

        uint16_t net_msg_id = 0;
        memcpy(&net_msg_id, data + nOffset, sizeof(net_msg_id));
        msg_id_ = SQUICK_NTOHS(net_msg_id);
        nOffset += sizeof(msg_id_);

        uint32_t nPackSize = 0;
        memcpy(&nPackSize, data + nOffset, sizeof(nPackSize));
        size_ = SQUICK_NTOHL(nPackSize) - SQUICK_HEAD_LENGTH;
        nOffset += sizeof(nPackSize);

        if (nOffset != SQUICK_HEAD_LENGTH) {
            assert(0);
        }

        return nOffset;
    }

    virtual uint16_t GetMsgID() const { return msg_id_; }

    virtual void SetMsgID(uint16_t msg_id) { msg_id_ = msg_id; }

    virtual uint32_t GetBodyLength() const { return size_; }

    virtual void SetBodyLength(uint32_t length) { size_ = length; }

  protected:
    uint16_t msg_id_;
    uint32_t size_;
    uint16_t hashcode_;
};

class INet;

typedef std::function<void(const socket_t sock, const int msg_id, const char *msg, const uint32_t len)> NET_RECEIVE_FUNCTOR;
typedef std::shared_ptr<NET_RECEIVE_FUNCTOR> NET_RECEIVE_FUNCTOR_PTR;

typedef std::function<void(const socket_t sock, const SQUICK_NET_EVENT nEvent, INet *pNet)> NET_EVENT_FUNCTOR;
typedef std::shared_ptr<NET_EVENT_FUNCTOR> NET_EVENT_FUNCTOR_PTR;

typedef std::function<void(int severity, const char *msg)> NET_EVENT_LOG_FUNCTOR;
typedef std::shared_ptr<NET_EVENT_LOG_FUNCTOR> NET_EVENT_LOG_FUNCTOR_PTR;

typedef std::function<Coroutine<bool>(const socket_t sock, const int msg_id, const char* msg, const uint32_t len)> NET_CORO_RECEIVE_FUNCTOR;
typedef std::shared_ptr<NET_CORO_RECEIVE_FUNCTOR> NET_CORO_RECEIVE_FUNCTOR_PTR;

class NetObject {
  public:
    NetObject(INet *pNet, socket_t sock, sockaddr_in &addr, void *pBev) {
        logicState = 0;
        fd = sock;
        bNeedRemove = false;
        netObject = pNet;
        userData = pBev;
        memset(&sin, 0, sizeof(sin));
        sin = addr;
        ip = inet_ntoa(addr.sin_addr);
    }

    virtual ~NetObject() {}

    int AddBuff(const char *str, size_t len) {
        ringBuff.append(str, len);

        return (int)ringBuff.length();
    }

    int CopyBuffTo(char *str, uint32_t start, uint32_t len) {
        if (start + len > ringBuff.length()) {
            return 0;
        }

        memcpy(str, ringBuff.data() + start, len);

        return len;
    }

    int RemoveBuff(uint32_t start, uint32_t len) {
        if (start + len > ringBuff.length()) {
            return 0;
        }

        ringBuff.erase(start, len);

        return (int)ringBuff.length();
    }

    const char *GetBuff() { return ringBuff.data(); }
    int GetBuffLen() const { return (int)ringBuff.length(); }
    void *GetUserData() { return userData; }
    INet *GetNet() { return netObject; }
    int GetConnectState() const { return logicState; }
    void SetConnectState(const int state) { logicState = state; }
    bool NeedRemove() { return bNeedRemove; }
    void SetNeedRemove(bool b) { bNeedRemove = b; }
    socket_t GetRealFD() { return fd; }

    string GetIP() {
        return ip;
    }

  private:
    sockaddr_in sin;
    void *userData;
    // ringbuff
    std::string ringBuff;
    int32_t logicState;
    INet *netObject;
    socket_t fd;
    bool bNeedRemove;
    std::string ip;
};

class INet {
  public:
    virtual ~INet() {}

    // need to call this function every frame to drive network library
    virtual bool Update() = 0;

    // as client
    virtual void Connect(const char *ip, const unsigned short nPort, const uint32_t expand_buffer_size) = 0;

    // as server
    virtual int Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount, const uint32_t expand_buffer_size) = 0;

    virtual bool Final() = 0;

    // send a message with out msg-head[auto add msg-head in this function]
    virtual bool SendMsg(const int16_t msg_id, const char *msg, const size_t len, const socket_t sock = 0) = 0;

    // send a message with out msg-head[need to add msg-head for this message by youself]
    virtual bool SendData(const char *msg, const size_t len, const socket_t sock) = 0;

    // send a data to all client[need to add msg-head for this message by youself]
    virtual bool SendDataToAllClient(const char *msg, const size_t len) = 0;

    // send a message with out msg-head to all client[auto add msg-head in this function]
    virtual bool SendMsgToAllClient(const int16_t msg_id, const char *msg, const size_t len) = 0;

    virtual bool CloseNetObject(const socket_t sock) = 0;

    virtual NetObject *GetNetObject(const socket_t sock) = 0;

    virtual bool AddNetObject(const socket_t sock, NetObject *pObject) = 0;

    virtual bool IsServer() = 0;

    virtual bool Log(int severity, const char *msg) = 0;
};