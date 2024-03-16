

#ifndef SQUICK_INTF_HTTP_NET_MODULE_H
#define SQUICK_INTF_HTTP_NET_MODULE_H

#include "i_http_server.h"
#include <squick/core/i_module.h>
#include <squick/core/platform.h>
#include "coroutine.h"

#define HTTP_SERVER_COROTINE_MAX_SURVIVAL_TIME 10

class IHttpServerModule : public IModule {
  public:
    virtual ~IHttpServerModule(){};

    // register msg callback
    template <typename BaseType>
    bool AddRequestHandler(const std::string &strPath, const HttpType eRequestType, BaseType *pBase,
                           bool (BaseType::*handleReceiver)(std::shared_ptr<HttpRequest> req)) {
        HTTP_RECEIVE_FUNCTOR functor = std::bind(handleReceiver, pBase, std::placeholders::_1);
        HTTP_RECEIVE_FUNCTOR_PTR functorPtr(new HTTP_RECEIVE_FUNCTOR(functor));
        return AddMsgCB(strPath, eRequestType, functorPtr);
    }

    template <typename BaseType>
    bool AddRequestHandler(const std::string& strPath, const HttpType eRequestType, BaseType* pBase,
        Coroutine<bool> (BaseType::* handleReceiver)(std::shared_ptr<HttpRequest> req)) {
        HTTP_RECEIVE_CORO_FUNCTOR functor = std::bind(handleReceiver, pBase, std::placeholders::_1);
        HTTP_RECEIVE_CORO_FUNCTOR_PTR functorPtr(new HTTP_RECEIVE_CORO_FUNCTOR(functor));
        return AddMsgCB(strPath, eRequestType, functorPtr);
    }

    template <typename BaseType>
    bool AddNetFilter(const std::string &strPath, BaseType *pBase, WebStatus (BaseType::*handleFilter)(std::shared_ptr<HttpRequest> req)) {
        HTTP_FILTER_FUNCTOR functor = std::bind(handleFilter, pBase, std::placeholders::_1);
        HTTP_FILTER_FUNCTOR_PTR functorPtr(new HTTP_FILTER_FUNCTOR(functor));

        return AddFilterCB(strPath, functorPtr);
    }

    template <typename BaseType>
    bool AddMiddleware(BaseType* pBase, WebStatus(BaseType::* handleFilter)(std::shared_ptr<HttpRequest> req)) {
        HTTP_FILTER_FUNCTOR functor = std::bind(handleFilter, pBase, std::placeholders::_1);
        HTTP_FILTER_FUNCTOR_PTR functorPtr(new HTTP_FILTER_FUNCTOR(functor));

        return AddMiddlewareCB(functorPtr);
    }

  public:
    virtual int StartServer(const unsigned short nPort) = 0;
    virtual bool ResponseMsg(std::shared_ptr<HttpRequest> req, const std::string &msg, WebStatus code = WebStatus::WEB_OK,
                             const std::string &reason = "OK") = 0;
    virtual bool SetHeader(std::shared_ptr<HttpRequest> req, const std::string& key, const std::string& value) = 0;
    virtual bool AddMiddlewareCB(const HTTP_FILTER_FUNCTOR_PTR& cb) = 0;
  private:
    virtual bool AddMsgCB(const std::string &strPath, const HttpType eRequestType, const HTTP_RECEIVE_FUNCTOR_PTR &cb) = 0;
    virtual bool AddMsgCB(const std::string& strPath, const HttpType eRequestType, const HTTP_RECEIVE_CORO_FUNCTOR_PTR& cb) = 0;
    virtual bool AddFilterCB(const std::string &strPath, const HTTP_FILTER_FUNCTOR_PTR &cb) = 0;
};
#endif
