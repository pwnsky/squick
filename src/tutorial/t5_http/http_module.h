#pragma once

#include <iostream>
#include <squick/core/base.h>
#include <squick/plugin/net/export.h>
namespace tutorial {
    class IHttpModule : public IModule {};
    class HttpModule : public IHttpModule {
    public:
        HttpModule(IPluginManager* p) {
            is_update_ = true;
            pm_ = p;
        }

        virtual bool Start();
        virtual bool AfterStart();
        virtual bool Update();
    protected:
        bool ShowRequestInfo(std::shared_ptr<HttpRequest> req);
        WebStatus OnFilter(std::shared_ptr<HttpRequest> req);
        
        // http client some examples
        bool ClientSyncGet(std::shared_ptr<HttpRequest> req);
        Coroutine<bool> ClientAsyncGet(std::shared_ptr<HttpRequest> req);
        bool ClientAsyncGet2(std::shared_ptr<HttpRequest> req);
        Coroutine<bool> ClientAsyncPost(std::shared_ptr<HttpRequest> req);

        bool PostListener(std::shared_ptr<HttpRequest> req);
        bool ClientPostSelf(std::shared_ptr<HttpRequest> req);
        void OnPostToServerCallBack(const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData);

        // A coroutine env in this query
        Coroutine<bool> OnCoroutineQuery(std::shared_ptr<HttpRequest> req);
        
    private:
        IHttpServerModule* m_http_server_;
        IHttpClientModule* m_http_client_;
    };
}