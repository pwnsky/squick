#pragma once

#include <assert.h>
#include <cstring>
#include <errno.h>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "coroutine.h"
#include "i_net.h"
#include <core/platform.h>

#if PLATFORM == PLATFORM_WIN
#include <fcntl.h>
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

enum WebStatus {
    WEB_OK = 200,
    WEB_AUTH = 401,
    WEB_ERROR = 404,
    WEB_INTER_ERROR = 500,
    WEB_TIMEOUT = 503,
    WEB_IGNORE = 1000, // Ignore
    WEB_RETURN = 1000, // Return
};

enum HttpType {
    SQUICK_HTTP_REQ_GET = 1 << 0,
    SQUICK_HTTP_REQ_POST = 1 << 1,
    SQUICK_HTTP_REQ_HEAD = 1 << 2,
    SQUICK_HTTP_REQ_PUT = 1 << 3,
    SQUICK_HTTP_REQ_DELETE = 1 << 4,
    SQUICK_HTTP_REQ_OPTIONS = 1 << 5,
    SQUICK_HTTP_REQ_TRACE = 1 << 6,
    SQUICK_HTTP_REQ_CONNECT = 1 << 7,
    SQUICK_HTTP_REQ_PATCH = 1 << 8
};

enum class HttpContentType {
    Html,
    Css,
    JavaScript,
    Json,
    Text,
    FontWoff,
    FontWoff2,
    Mp3,
    XMsvideo,
    jpeg,
    Png,
    XIcon,
    Binary,
};

class HttpRequest {
  public:
    HttpRequest(const int64_t index) {
        id = index;
        Reset();
    }

    void Reset() {
        url.clear();
        path.clear();
        remoteHost.clear();
        // type
        body.clear();
        params.clear();
        headers.clear();
    }
    int64_t id;
    void *req;
    std::string url;
    std::string path;
    std::string remoteHost;
    HttpType type;
    std::string body;                          // when using post
    std::map<std::string, std::string> params; // when using get
    std::map<std::string, std::string> headers;
};

// it should be
typedef std::function<bool(std::shared_ptr<HttpRequest> req)> HTTP_RECEIVE_FUNCTOR;
typedef std::shared_ptr<HTTP_RECEIVE_FUNCTOR> HTTP_RECEIVE_FUNCTOR_PTR;

typedef std::function<WebStatus(std::shared_ptr<HttpRequest> req)> HTTP_FILTER_FUNCTOR;
typedef std::shared_ptr<HTTP_FILTER_FUNCTOR> HTTP_FILTER_FUNCTOR_PTR;

typedef std::function<Coroutine<bool>(std::shared_ptr<HttpRequest> req)> HTTP_RECEIVE_CORO_FUNCTOR;
typedef std::shared_ptr<HTTP_RECEIVE_CORO_FUNCTOR> HTTP_RECEIVE_CORO_FUNCTOR_PTR;

class IHttpServer {
  public:
    virtual ~IHttpServer() {}

    virtual bool Update() = 0;

    virtual int StartServer(const unsigned short port, bool is_ssl = false) = 0;

    virtual bool ResponseMsg(std::shared_ptr<HttpRequest> req, const std::string &msg, WebStatus code, const std::string &strReason = "OK") = 0;
    virtual bool SetHeader(std::shared_ptr<HttpRequest> req, const std::string &key, const std::string &value) = 0;

    virtual std::shared_ptr<HttpRequest> GetHttpRequest(const int64_t index) = 0;
};
