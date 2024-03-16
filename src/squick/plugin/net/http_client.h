
#ifndef SQUICK_HTTP_CLIENT_H
#define SQUICK_HTTP_CLIENT_H

#include "i_http_client.h"
#include "i_http_server.h"
#include "coroutine.h"

#if PLATFORM == PLATFORM_WIN
#include <fcntl.h>
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x)&S_IFMT) == S_IFDIR)
#endif

#else

#include <atomic>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/bufferevent_struct.h>
#include <event2/event.h>
#include <event2/event_compat.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event2/util.h>

#include <map>

// it would be a pool
class HttpObject {
  public:
    HttpObject(IHttpClient *pNet, struct bufferevent *pBev, HTTP_RESP_FUNCTOR_PTR pCB, Guid id, const std::string aMemo = "") {
        m_pBev = pBev;
        m_pHttpClient = pNet;
        m_pCB = pCB;

        mID = id;
        strMemo = aMemo;
    }

    virtual ~HttpObject() {}

    Guid mID;
    bufferevent *m_pBev;
    IHttpClient *m_pHttpClient;
    HTTP_RESP_FUNCTOR_PTR m_pCB;
    std::string strMemo;
};

class HttpClient : public IHttpClient {
  public:
    HttpClient(int nRetry = 2, int nTimeoutSec = 30) : m_nRetry(nRetry), m_nTimeOut(nTimeoutSec) {}

    virtual ~HttpClient(){};

  public:
    virtual bool Update();

    virtual bool Start();

    virtual bool Final();

    virtual bool DoGet(const std::string &strUri, HTTP_RESP_FUNCTOR_PTR pCB, const std::map<std::string, std::string> &xHeaders, const Guid id = Guid());

    virtual bool DoPost(const std::string &strUri, const std::string &strPostData, const std::string &strMemoData, HTTP_RESP_FUNCTOR_PTR pCB,
                        const std::map<std::string, std::string> &xHeaders, const Guid id = Guid());

    virtual Awaitable<HttpClientResponseData> Get(const std::string& url, const std::map<std::string, std::string>& xHeaders, const Guid id = Guid()) override;

  private:
    static void OnHttpReqDone(struct evhttp_request *req, void *ctx);
    void CoroutineBinder(Awaitable<HttpClientResponseData>* http_await);
    void CoroutineResponseHandler(const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData);

    bool MakeRequest(const std::string &strUri, HTTP_RESP_FUNCTOR_PTR pCB, const std::string &strPostData, const std::map<std::string, std::string> &xHeaders,
                     const HttpType eHttpType, const std::string &strMemoData, const Guid id = Guid());
    
    reqid_t GenerateRequestID();
  private:
    std::string m_strUserAgent;
    struct event_base *m_pBase = nullptr;

    int m_nRetry = 2;
    int m_nTimeOut = 30;

    std::list<HttpObject *> mlHttpObject;
    reqid_t  last_req_id_ = 0;
    std::map<reqid_t, Awaitable<HttpClientResponseData>*> co_awaitbles_;
#if SQUICK_ENABLE_SSL
    SSL_CTX *m_pSslCtx = nullptr;
#endif
};

#endif
