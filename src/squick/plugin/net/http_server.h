

#ifndef SQUICK_HTTP_SERVER_H
#define SQUICK_HTTP_SERVER_H

#include "i_http_server.h"
#include <squick/core/map_ex.h>

#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif

#else

#include <squick/core/exception.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <atomic>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#endif


#include <event2/bufferevent.h>
#include "event2/bufferevent_struct.h"
#include "event2/event.h"
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <event2/http_struct.h>
#include <event2/rpc.h>


class HttpServer : public IHttpServer
{
public:
    HttpServer()
    {
    }

    template<typename BaseType>
    HttpServer(BaseType* pBaseType, bool (BaseType::*handleReceiver)(SQUICK_SHARE_PTR<HttpRequest> req), WebStatus (BaseType::*handleFilter)(SQUICK_SHARE_PTR<HttpRequest> req))
    {
        mxBase = NULL;
		mReceiveCB = std::bind(handleReceiver, pBaseType, std::placeholders::_1);
		mFilter = std::bind(handleFilter, pBaseType, std::placeholders::_1);
    }

    virtual ~HttpServer()
	{
		if (mxBase)
		{
			event_base_free(mxBase);
			mxBase = NULL;
		}

        mxHttpRequestMap.ClearAll();
        mxHttpRequestPool.clear();
	};


    virtual bool Update();

    virtual int StartServer(const unsigned short nPort);

    virtual bool ResponseMsg(SQUICK_SHARE_PTR<HttpRequest> req, const std::string& msg, WebStatus code, const std::string& strReason = "OK");

    virtual SQUICK_SHARE_PTR<HttpRequest> GetHttpRequest(const int64_t index);

private:
    static void listener_cb(struct evhttp_request* req, void* arg);

	SQUICK_SHARE_PTR<HttpRequest> AllocHttpRequest();

private:
    int64_t mIndex = 0;

    struct event_base* mxBase;
	HTTP_RECEIVE_FUNCTOR mReceiveCB;
	HTTP_FILTER_FUNCTOR mFilter;

	MapEx<int64_t, HttpRequest> mxHttpRequestMap;
	std::list<SQUICK_SHARE_PTR<HttpRequest>> mxHttpRequestPool;
};

#endif
