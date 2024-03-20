
#include "http_module.h"
/*
* A example for useage of http server
* How to start work?
* ./squick type=tutorial/t5_http
*/
namespace tutorial {
    bool HttpModule::Start() {
        m_http_client_ = pm_->FindModule<IHttpClientModule>();
        m_http_server_ = pm_->FindModule<IHttpServerModule>();
        return true;
    }

    bool HttpModule::AfterStart() {
        m_http_server_->AddRequestHandler("/", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::ShowRequestInfo);
        m_http_server_->AddRequestHandler("/", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::ShowRequestInfo);
        m_http_server_->AddNetFilter("/", this, &HttpModule::OnFilter);
        
        m_http_server_->AddRequestHandler("/coro", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnCoroutineQuery);
        
        m_http_server_->AddRequestHandler("/client/sync_get", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::ClientSyncGet);
        m_http_server_->AddRequestHandler("/client/async_get", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::ClientAsyncGet);
        m_http_server_->AddRequestHandler("/client/async_get_2", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::ClientAsyncGet2);
        m_http_server_->AddRequestHandler("/client/async_post", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::ClientAsyncPost);

        m_http_server_->AddRequestHandler("/post_listener", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::PostListener);
        m_http_server_->AddRequestHandler("/client/post_self", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::ClientPostSelf);
        
        m_http_server_->StartServer(8080);

        std::cout << "Tutorial HttpModule Started\nThis is a example for learning http server.\n"
            << "Now you can use browser to request url: http://127.0.0.1:8080\n"
            << std::endl;
        return true;
    }

    bool HttpModule::Update() {
        if (m_http_server_)
            m_http_server_->Update();
        return true;
    }

    Coroutine<bool> HttpModule::OnCoroutineQuery(std::shared_ptr<HttpRequest> req) {
        m_http_server_->ResponseMsg(req, "You query a coroutine function", WebStatus::WEB_OK);
        co_return;
    }

    bool HttpModule::ClientSyncGet(std::shared_ptr<HttpRequest> req) {
        std::cout << "On sync get, this will block server when request https://bilibili.com \n";
        IHttpServerModule* m = this->m_http_server_;
        string data;
        m_http_client_->Get("https://www.bilibili.com", data);
        m->ResponseMsg(req, data, WebStatus::WEB_OK);
        return true;
    }

    Coroutine<bool> HttpModule::ClientAsyncGet(std::shared_ptr<HttpRequest> req) {
        std::cout << "You use await to async request another server\n";
        auto data = co_await m_http_client_->CoGet("http://www.baidu.com");
        m_http_server_->ResponseMsg(req, data.content, WebStatus::WEB_OK);
        co_return;
    }

    bool HttpModule::ClientAsyncGet2(std::shared_ptr<HttpRequest> req) {
        std::cout << "OnClientGet: request https://bilibili.com \n";
        IHttpServerModule* m = this->m_http_server_;
        m_http_client_->DoGet("https://www.bilibili.com",
            [m, req](const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData) -> void {
                m->ResponseMsg(req, strRespData, WebStatus::WEB_OK);
            });
        return true;
    }

    Coroutine<bool> HttpModule::ClientAsyncPost(std::shared_ptr<HttpRequest> req) {
        std::cout << "You use await to async request another server: method POST\n";
        auto data = co_await m_http_client_->CoPost("http://127.0.0.1:8080/post_listener", "hello???");
        m_http_server_->ResponseMsg(req, data.content, WebStatus::WEB_OK);
        std::cout << "You get content: " << data.content << "\n";
        co_return;
    }

    bool HttpModule::ShowRequestInfo(std::shared_ptr<HttpRequest> req) {
        std::cout << "url: " << req->url << std::endl;
        std::cout << "path: " << req->path << std::endl;
        std::cout << "type: " << req->type << std::endl;
        std::cout << "body: " << req->body << std::endl;

        std::cout << "params: " << std::endl;
        for (auto item : req->params) {
            std::cout << item.first << ":" << item.second << std::endl;
        }

        std::cout << "headers: " << std::endl;
        for (auto item : req->headers) {
            std::cout << item.first << ":" << item.second << std::endl;
        }

        return m_http_server_->ResponseMsg(req, "{\"code\": 0}", WebStatus::WEB_OK);
    }

    WebStatus HttpModule::OnFilter(std::shared_ptr<HttpRequest> req) {
        std::cout << "OnFilter ... " << std::endl;
        // You can do something before handle function.
        return WebStatus::WEB_OK;
    }

    bool HttpModule::PostListener(std::shared_ptr<HttpRequest> req) {
        std::cout << "PostListener: Post data: " << req->body << endl;
        m_http_server_->ResponseMsg(req, "PostListener has received!: " + req->body, WebStatus::WEB_OK);
        return true;
    }

    bool HttpModule::ClientPostSelf(std::shared_ptr<HttpRequest> req) {
        std::string strMemo = "Memo here";
        m_http_client_->DoPost("http://127.0.0.1:8080/json", "OnHeartBeat post data---", this, &HttpModule::OnPostToServerCallBack, strMemo);
        
        m_http_client_->DoPost("http://127.0.0.1:8080/json", "OnHeartBeat post data---",
            [](const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData) -> void {
                std::cout << "OnPostToServerCallBack2" << std::endl;
            });
        m_http_server_->ResponseMsg(req, "Data posted", WebStatus::WEB_OK);
        return 0;
    }

    void HttpModule::OnPostToServerCallBack(const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData) {
        std::cout << "OnPostToServerCallBack1"
            << " " << strMemoData << std::endl;
    }
}