

#ifndef NFREDISPLUGIN_NFREDISCLIENTSOCKET_H
#define NFREDISPLUGIN_NFREDISCLIENTSOCKET_H

#include <atomic>
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
#include <vector>

#include <squick/core/guid.h>

#if PLATFORM == PLATFORM_WIN

#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_LINUX || PLATFORM == PLATFORM_ANDROID

#if PLATFORM == PLATFORM_APPLE

#include <libkern/OSByteOrder.h>

#endif

#include <netinet/in.h>

#ifdef _XOPEN_SOURCE_EXTENDED
#include <arpa/inet.h>
#endif

#include <sys/socket.h>
#include <unistd.h>

#endif

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event2/util.h>
#if PLATFORM == PLATFORM_WIN
#include <hiredis_win/hiredis.h>
#else
#include <hiredis/hiredis.h>
#endif

class RedisClientSocket {
  public:
    enum SQUICK_NET_EVENT {
        SQUICK_NET_EVENT_NONE = 0,
        SQUICK_NET_EVENT_EOF = 0x10,
        SQUICK_NET_EVENT_ERROR = 0x20,
        SQUICK_NET_EVENT_TIMEOUT = 0x40,
        SQUICK_NET_EVENT_CONNECTED = 0x80,
    };

    RedisClientSocket();
    virtual ~RedisClientSocket();

    int64_t Connect(const std::string &ip, const int port);
    bool ReConnect(const std::string &ip, const int port);
    int Close();
    int Write(const char *buf, size_t count);

    int Update();

    bool IsConnect();
    redisReader *GetRedisReader();

  protected:
    static void listener_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sa, int socklen, void *user_data);
    static void conn_readcb(struct bufferevent *bev, void *user_data);
    static void conn_writecb(struct bufferevent *bev, void *user_data);
    static void conn_eventcb(struct bufferevent *bev, short events, void *user_data);
    static void log_cb(int severity, const char *msg);

    string GetIP(const std::string &url);

  private:
    struct event_base *base;
    struct bufferevent *bev;
    struct evconnlistener *listener;

    SQUICK_NET_EVENT mNetStatus;
    redisReader *m_pRedisReader;
    INT64 fd_;
};

#endif // NFREDISPLUGIN_NFREDISCLIENTSOCKET_H
