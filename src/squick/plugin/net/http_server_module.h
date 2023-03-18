#pragma once

#include "squick/core/map_ex.h"
#include "squick/core/performance.h"
#include <iostream>

#include "i_http_server.h"
#include "i_http_server_module.h"
#include <squick/plugin/log/i_log_module.h>

class HttpServerModule : public IHttpServerModule {

  public:
    HttpServerModule(IPluginManager *p);

    virtual ~HttpServerModule();

  public:
    virtual int StartServer(const unsigned short nPort);

    virtual bool Update();

    virtual bool ResponseMsg(SQUICK_SHARE_PTR<HttpRequest> req, const std::string &msg, WebStatus code = WebStatus::WEB_OK, const std::string &reason = "OK");

  private:
    virtual bool OnReceiveNetPack(SQUICK_SHARE_PTR<HttpRequest> req);
    virtual WebStatus OnFilterPack(SQUICK_SHARE_PTR<HttpRequest> req);

    virtual bool AddMsgCB(const std::string &strCommand, const HttpType eRequestType, const HTTP_RECEIVE_FUNCTOR_PTR &cb);
    virtual bool AddFilterCB(const std::string &strCommand, const HTTP_FILTER_FUNCTOR_PTR &cb);

  private:
    IHttpServer *m_pHttpServer;
    ILogModule *m_pLogModule;

    MapEx<HttpType, std::map<std::string, HTTP_RECEIVE_FUNCTOR_PTR>> mMsgCBMap;
    std::map<std::string, HTTP_FILTER_FUNCTOR_PTR> mMsgFliterMap;

    HTTP_RECEIVE_FUNCTOR_PTR mComMsgCBList;
};