#pragma once

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


enum WebStatus
{
    WEB_OK = 200,
	WEB_AUTH = 401,
	WEB_ERROR = 404,
	WEB_INTER_ERROR = 500,
    WEB_TIMEOUT = 503,
};

enum HttpType
{
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

class HttpRequest
{
public:
	HttpRequest(const int64_t index)
	{
		id = index;
		Reset();
	}

	void Reset()
	{
		url.clear();
		path.clear();
		remoteHost.clear();
		//type
		body.clear();
		params.clear();
		headers.clear();
	}
	int64_t id;
    void* req;
	std::string url;
	std::string path;
	std::string remoteHost;
	HttpType type;
	std::string body;//when using post
	std::map<std::string, std::string> params;//when using get
	std::map<std::string, std::string> headers;
};

//it should be
typedef std::function<bool(SQUICK_SHARE_PTR<HttpRequest> req)> HTTP_RECEIVE_FUNCTOR;
typedef std::shared_ptr<HTTP_RECEIVE_FUNCTOR> HTTP_RECEIVE_FUNCTOR_PTR;

typedef std::function<WebStatus(SQUICK_SHARE_PTR<HttpRequest> req)> HTTP_FILTER_FUNCTOR;
typedef std::shared_ptr<HTTP_FILTER_FUNCTOR> HTTP_FILTER_FUNCTOR_PTR;

class IHttpServer
{
public:
	virtual ~IHttpServer(){}
	
    virtual bool Update() = 0;

    virtual int StartServer(const unsigned short nPort) = 0;

    virtual bool ResponseMsg(SQUICK_SHARE_PTR<HttpRequest> req, const std::string& msg, WebStatus code, const std::string& strReason = "OK") = 0;

    virtual SQUICK_SHARE_PTR<HttpRequest> GetHttpRequest(const int64_t index) = 0;
};

