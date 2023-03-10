
#ifndef SQUICK_INTF_HTTP_CLIENT_H
#define SQUICK_INTF_HTTP_CLIENT_H

#include <cstring>
#include <string>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <list>
#include <vector>
#include <assert.h>

#include "i_net.h"
#include <squick/core/platform.h>

#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <fcntl.h>
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#if SQUICK_ENABLE_SSL

#include <event2/bufferevent_ssl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#endif

typedef std::function<void(const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData)> HTTP_RESP_FUNCTOR;
typedef std::shared_ptr<HTTP_RESP_FUNCTOR> HTTP_RESP_FUNCTOR_PTR;

class IHttpClient
{
public:
    virtual ~IHttpClient()
    {}

    virtual bool Update() = 0;

    virtual bool Start() = 0;

    virtual bool Final() = 0;

    virtual bool DoGet(const std::string& strUri, HTTP_RESP_FUNCTOR_PTR pCB,
                            const std::map<std::string, std::string>& xHeaders, const Guid id = Guid()) = 0;

    virtual bool DoPost(const std::string& strUri, const std::string& strPostData, const std::string& strMemoData, HTTP_RESP_FUNCTOR_PTR pCB,
                             const std::map<std::string, std::string>& xHeaders, const Guid id = Guid()) = 0;


};

#endif
