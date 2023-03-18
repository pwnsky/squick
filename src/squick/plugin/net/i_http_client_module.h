#pragma once

#include "i_http_client.h"
#include <squick/core/i_module.h>

class IHttpClientModule : public IModule {
  public:
    virtual ~IHttpClientModule(){};

    template <typename BaseType>
    bool DoGet(const std::string &strUri, BaseType *pBase,
               void (BaseType::*handleReceiver)(const Guid id, const int state_code, const std::string &strRespData)) {
        HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
        return DoGet(strUri, std::map<std::string, std::string>(), pd);
    }

    bool DoGet(const std::string &strUri, HTTP_RESP_FUNCTOR functor) {
        HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(functor));
        return DoGet(strUri, std::map<std::string, std::string>(), pd);
    }

    template <typename BaseType>
    bool DoGet(const std::string &strUri, const std::map<std::string, std::string> &xHeaders, BaseType *pBase,
               void (BaseType::*handleReceiver)(const Guid id, const int state_code, const std::string &strRespData)) {
        HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
        return DoGet(strUri, xHeaders, pd);
    }

    bool DoGet(const std::string &strUri, const std::map<std::string, std::string> &xHeaders, HTTP_RESP_FUNCTOR functor) {
        HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(functor));
        return DoGet(strUri, xHeaders, pd);
    }

    template <typename BaseType>
    bool DoPost(const std::string &strUri, const std::string &strPostData, BaseType *pBase,
                void (BaseType::*handleReceiver)(const Guid id, const int state_code, const std::string &strRespData, const std::string &strMemo),
                const std::string &strMemo = "") {
        HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(
            std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
        return DoPost(strUri, std::map<std::string, std::string>(), strPostData, pd, strMemo);
    }

    bool DoPost(const std::string &strUri, const std::string &strPostData, HTTP_RESP_FUNCTOR functor, const std::string &strMemo = "") {
        HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(functor));
        return DoPost(strUri, std::map<std::string, std::string>(), strPostData, pd, strMemo);
    }

    template <typename BaseType>
    bool DoPost(const std::string &strUri, const std::string &strPostData, const std::map<std::string, std::string> &xHeaders, BaseType *pBase,
                void (BaseType::*handleReceiver)(const Guid id, const int state_code, const std::string &strRespData), const std::string &strMemo = "") {
        HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(std::bind(handleReceiver, pBase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
        return DoPost(strUri, xHeaders, strPostData, pd, strMemo);
    }

    bool DoPost(const std::string &strUri, const std::string &strPostData, const std::map<std::string, std::string> &xHeaders, HTTP_RESP_FUNCTOR functor,
                const std::string &strMemo = "") {
        HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(functor));
        return DoPost(strUri, xHeaders, strPostData, pd, strMemo);
    }
    // actually, sync post method
    // WebStatus
    virtual int Post(const std::string &strUri, const std::string &strData, std::string &strResData) = 0;
    virtual int Post(const std::string &strUri, const std::map<std::string, std::string> &xHeaders, const std::string &strData, std::string &strResData) = 0;

    // actually, sync get method
    // WebStatus
    virtual int Get(const std::string &strUri, std::string &strResData) = 0;
    virtual int Get(const std::string &strUri, const std::map<std::string, std::string> &xHeaders, std::string &strResData) = 0;

  protected:
    virtual bool DoGet(const std::string &strUrl, const std::map<std::string, std::string> &xHeaders, HTTP_RESP_FUNCTOR_PTR pCB) = 0;

    virtual bool DoPost(const std::string &strUrl, const std::map<std::string, std::string> &xHeaders, const std::string &strPostData,
                        HTTP_RESP_FUNCTOR_PTR pCB, const std::string &strMemo) = 0;
};