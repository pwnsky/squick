

#ifndef SQUICK_HTTP_SERVER_H
#define SQUICK_HTTP_SERVER_H

#include "i_http_server.h"
#include <core/map_ex.h>

#if PLATFORM == PLATFORM_WIN
#include <fcntl.h>
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif

#else

#include <atomic>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <core/exception.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#endif

// #include "event2/bufferevent_struct.h"
#include "event2/event.h"
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>
#include <event2/rpc.h>
#include <event2/util.h>

class HttpServer : public IHttpServer {
  public:
    HttpServer() {}

    template <typename BaseType>
    HttpServer(BaseType *pBaseType, bool (BaseType::*handleReceiver)(std::shared_ptr<HttpRequest> req),
               WebStatus (BaseType::*handleFilter)(std::shared_ptr<HttpRequest> req)) {
        mxBase = NULL;
        mReceiveCB = std::bind(handleReceiver, pBaseType, std::placeholders::_1);
        mFilter = std::bind(handleFilter, pBaseType, std::placeholders::_1);
    }

    virtual ~HttpServer() {
        if (mxBase) {
            event_base_free(mxBase);
            mxBase = NULL;
        }

        mxHttpRequestMap.ClearAll();
        mxHttpRequestPool.clear();
    };

    virtual bool Update();

    virtual int StartServer(const unsigned short port, bool is_ssl);

    virtual bool ResponseMsg(std::shared_ptr<HttpRequest> req, const std::string &msg, WebStatus code, const std::string &strReason = "OK");
    virtual bool SetHeader(std::shared_ptr<HttpRequest> req, const std::string &key, const std::string &value);

    virtual std::shared_ptr<HttpRequest> GetHttpRequest(const int64_t index);

  private:
    static void listener_cb(struct evhttp_request *req, void *arg);

    std::shared_ptr<HttpRequest> AllocHttpRequest();

  private:
    int64_t mIndex = 0;

    struct event_base *mxBase;
    HTTP_RECEIVE_FUNCTOR mReceiveCB;
    HTTP_FILTER_FUNCTOR mFilter;

    MapEx<int64_t, HttpRequest> mxHttpRequestMap;
    std::list<std::shared_ptr<HttpRequest>> mxHttpRequestPool;
};

#endif
