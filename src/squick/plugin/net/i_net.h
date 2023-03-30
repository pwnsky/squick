
#ifndef SQUICK_INTF_NET_H
#define SQUICK_INTF_NET_H

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

#if PLATFORM == PLATFORM_WIN
#include <WinSock2.h>
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_LINUX || PLATFORM == PLATFORM_ANDROID

#if PLATFORM == PLATFORM_APPLE

#include <libkern/OSByteOrder.h>

#endif

#ifdef _XOPEN_SOURCE_EXTENDED
#include <arpa/inet.h>
#endif

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

#pragma pack(push, 1)

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

    static int64_t SQUICK_HTONLL(int64_t nData) {
#if PLATFORM == PLATFORM_WIN
        return htonll(nData);
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
        return OSSwapHostToBigInt64(nData);
#else
        return htobe64(nData);
#endif
    }

    static int64_t SQUICK_NTOHLL(int64_t nData) {
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

    static int32_t SQUICK_HTONL(int32_t nData) {
#if PLATFORM == PLATFORM_WIN
        return htonl(nData);
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
        return OSSwapHostToBigInt32(nData);
#else
        return htobe32(nData);
#endif
    }

    static int32_t SQUICK_NTOHL(int32_t nData) {
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

    static int16_t SQUICK_HTONS(int16_t nData) {
#if PLATFORM == PLATFORM_WIN
        return htons(nData);
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
        return OSSwapHostToBigInt16(nData);
#else
        return htobe16(nData);
#endif
    }

    static int16_t SQUICK_NTOHS(int16_t nData) {
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
class SquickStructHead : public IMsgHead {
  public:
    SquickStructHead() {
        munSize = 0;
        mumsg_id = 0;
    }

    // Message Head[ MsgID(2) | MsgSize(4) ]
    virtual int EnCode(char *data) {
        uint32_t nOffset = 0;

        uint16_t msg_id = SQUICK_HTONS(mumsg_id);
        memcpy(data + nOffset, (void *)(&msg_id), sizeof(mumsg_id));
        nOffset += sizeof(mumsg_id);

        uint32_t nPackSize = munSize + SQUICK_HEAD_LENGTH;
        uint32_t nSize = SQUICK_HTONL(nPackSize);
        memcpy(data + nOffset, (void *)(&nSize), sizeof(munSize));
        nOffset += sizeof(munSize);

        if (nOffset != SQUICK_HEAD_LENGTH) {
            assert(0);
        }

        return nOffset;
    }

    // Message Head[ MsgID(2) | MsgSize(4) ]
    virtual int DeCode(const char *data) {
        uint32_t nOffset = 0;

        uint16_t msg_id = 0;
        memcpy(&msg_id, data + nOffset, sizeof(mumsg_id));
        mumsg_id = SQUICK_NTOHS(msg_id);
        nOffset += sizeof(mumsg_id);

        uint32_t nPackSize = 0;
        memcpy(&nPackSize, data + nOffset, sizeof(munSize));
        munSize = SQUICK_NTOHL(nPackSize) - SQUICK_HEAD_LENGTH;
        nOffset += sizeof(munSize);

        if (nOffset != SQUICK_HEAD_LENGTH) {
            assert(0);
        }

        return nOffset;
    }

    virtual uint16_t GetMsgID() const { return mumsg_id; }

    virtual void SetMsgID(uint16_t msg_id) { mumsg_id = msg_id; }

    virtual uint32_t GetBodyLength() const { return munSize; }

    virtual void SetBodyLength(uint32_t length) { munSize = length; }

  protected:
    uint32_t munSize;
    uint16_t mumsg_id;
};

class INet;

typedef std::function<void(const socket_t sock, const int msg_id, const char *msg, const uint32_t len)> NET_RECEIVE_FUNCTOR;
typedef std::shared_ptr<NET_RECEIVE_FUNCTOR> NET_RECEIVE_FUNCTOR_PTR;

typedef std::function<void(const socket_t sock, const SQUICK_NET_EVENT nEvent, INet *pNet)> NET_EVENT_FUNCTOR;
typedef std::shared_ptr<NET_EVENT_FUNCTOR> NET_EVENT_FUNCTOR_PTR;

typedef std::function<void(int severity, const char *msg)> NET_EVENT_LOG_FUNCTOR;
typedef std::shared_ptr<NET_EVENT_LOG_FUNCTOR> NET_EVENT_LOG_FUNCTOR_PTR;

class NetObject {
  public:
    NetObject(INet *pNet, socket_t sock, sockaddr_in &addr, void *pBev) {
        logicState = 0;
        gameID = 0;
        fd = sock;
        bNeedRemove = false;

        netObject = pNet;

        userData = pBev;
        memset(&sin, 0, sizeof(sin));
        sin = addr;
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

    //////////////////////////////////////////////////////////////////////////
    const std::string &GetSecurityKey() const { return securityKey; }

    void SetSecurityKey(const std::string &key) { securityKey = key; }

    int GetConnectKeyState() const { return logicState; }

    void SetConnectKeyState(const int state) { logicState = state; }

    bool NeedRemove() { return bNeedRemove; }

    void SetNeedRemove(bool b) { bNeedRemove = b; }

    const std::string &GetAccount() const { return account; }

    void SetAccount(const std::string &data) { account = data; }

    int GetGameID() const { return gameID; }

    void SetGameID(const int nData) { gameID = nData; }

    const Guid &GetUserID() { return userID; }

    void SetUserID(const Guid &nUserID) { userID = nUserID; }

    const Guid &GetClientID() { return clientID; }

    void SetClientID(const Guid &xClientID) { clientID = xClientID; }

    const Guid &GetHashIdentID() { return hashIdentID; }

    void SetHashIdentID(const Guid &xHashIdentID) { hashIdentID = xHashIdentID; }

    socket_t GetRealFD() { return fd; }

  private:
    sockaddr_in sin;
    void *userData;
    // ringbuff
    std::string ringBuff;
    std::string account;
    std::string securityKey;

    int32_t logicState;
    int32_t gameID;
    Guid userID;      // player id
    Guid clientID;    // temporary client id
    Guid hashIdentID; // hash ident, special for distributed
    INet *netObject;
    //
    socket_t fd;
    bool bNeedRemove;
};

class INet {
  public:
    virtual ~INet() {}

    // need to call this function every frame to drive network library
    virtual bool Update() = 0;

    // as client
    virtual void Startialization(const char *ip, const unsigned short nPort) = 0;

    // as server
    virtual int Startialization(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount = 4) = 0;

    virtual unsigned int ExpandBufferSize(const unsigned int size) = 0;

    virtual bool Final() = 0;

    // send a message with out msg-head[auto add msg-head in this function]
    virtual bool SendMsgWithOutHead(const int16_t msg_id, const char *msg, const size_t len, const socket_t sock = 0) = 0;

    // send a message with out msg-head[need to add msg-head for this message by youself]
    virtual bool SendMsg(const char *msg, const size_t len, const socket_t sock) = 0;

    // send a message to all client[need to add msg-head for this message by youself]
    virtual bool SendMsgToAllClient(const char *msg, const size_t len) = 0;

    // send a message with out msg-head to all client[auto add msg-head in this function]
    virtual bool SendMsgToAllClientWithOutHead(const int16_t msg_id, const char *msg, const size_t len) = 0;

    virtual bool CloseNetObject(const socket_t sock) = 0;

    virtual NetObject *GetNetObject(const socket_t sock) = 0;

    virtual bool AddNetObject(const socket_t sock, NetObject *pObject) = 0;

    virtual bool IsServer() = 0;

    virtual bool Log(int severity, const char *msg) = 0;
};

#pragma pack(pop)

#endif
