
#ifndef SQUICK_INTF_HTTP_CLIENT_H
#define SQUICK_INTF_HTTP_CLIENT_H

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

#include "i_net.h"
#include <squick/core/platform.h>
#include "coroutine.h"

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

#if SQUICK_ENABLE_SSL

#include <event2/bufferevent_ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>

#endif

typedef std::function<void(const Guid id, const int state_code, const std::string &strRespData, const std::string &strMemoData)> HTTP_RESP_FUNCTOR;
typedef std::shared_ptr<HTTP_RESP_FUNCTOR> HTTP_RESP_FUNCTOR_PTR;
typedef long int reqid_t;

struct HttpClientResponseData {
    reqid_t req_id;
    int error;
    int state_code;
    std::string header;
    std::string content;
};

class IHttpClient {
  public:
    virtual ~IHttpClient() {}

    virtual bool Update() = 0;

    virtual bool Start() = 0;

    virtual bool Final() = 0;

    virtual bool DoGet(const std::string &strUri, HTTP_RESP_FUNCTOR_PTR pCB, const std::map<std::string, std::string> &xHeaders, const Guid id = Guid()) = 0;

    virtual bool DoPost(const std::string &strUri, const std::string &strPostData, const std::string &strMemoData, HTTP_RESP_FUNCTOR_PTR pCB,
                        const std::map<std::string, std::string> &xHeaders, const Guid id = Guid()) = 0;

    virtual Awaitable<HttpClientResponseData> CoGet(const std::string& url, const std::map<std::string, std::string>& xHeaders) = 0;
    virtual Awaitable<HttpClientResponseData> CoPost(const std::string &url, const std::string &strPostData, const std::string &strMemoData,
                                                     const std::map<std::string, std::string> &xHeaders) = 0;

};

#endif
