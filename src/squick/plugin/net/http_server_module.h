#pragma once

#include "i_http_server.h"
#include "i_http_server_module.h"
#include "squick/core/map_ex.h"
#include "squick/core/performance.h"
#include <iostream>
#include <squick/core/base.h>
#include <squick/plugin/log/i_log_module.h>

#include <list>

class HttpServerModule : public IHttpServerModule {

  public:
    HttpServerModule(IPluginManager *p);

    virtual ~HttpServerModule();

  public:
    virtual int StartServer(const unsigned short nPort);

    virtual bool Update();

    virtual bool ResponseMsg(std::shared_ptr<HttpRequest> req, const std::string &msg, WebStatus code = WebStatus::WEB_OK, const std::string &reason = "OK");
    virtual bool SetHeader(std::shared_ptr<HttpRequest> req, const std::string &key, const std::string &value);

  private:
    virtual bool OnReceiveNetPack(std::shared_ptr<HttpRequest> req);
    virtual WebStatus OnFilterPack(std::shared_ptr<HttpRequest> req);
    virtual bool AddMsgCB(const std::string &strCommand, const HttpType eRequestType, const HTTP_RECEIVE_FUNCTOR_PTR &cb);
    virtual bool AddMsgCB(const std::string &strCommand, const HttpType eRequestType, const HTTP_RECEIVE_CORO_FUNCTOR_PTR &cb);
    virtual bool AddFilterCB(const std::string &strCommand, const HTTP_FILTER_FUNCTOR_PTR &cb);
    virtual bool AddMiddlewareCB(const HTTP_FILTER_FUNCTOR_PTR &cb);
    int FixCoroutines(time_t now_time);

  private:
    IHttpServer *m_pHttpServer;
    ILogModule *m_log_;

    MapEx<HttpType, std::map<std::string, HTTP_RECEIVE_FUNCTOR_PTR>> mMsgCBMap;
    MapEx<HttpType, std::map<std::string, HTTP_RECEIVE_CORO_FUNCTOR_PTR>> coro_funcs_;
    std::map<std::string, HTTP_FILTER_FUNCTOR_PTR> mMsgFliterMap;

    list<Coroutine<bool>> coroutines_;
    HTTP_RECEIVE_FUNCTOR_PTR mComMsgCBList;
    HTTP_FILTER_FUNCTOR_PTR middleware_ = nullptr;
    time_t last_check_coroutines_time_ = 0;
};