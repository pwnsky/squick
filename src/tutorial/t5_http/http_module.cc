
#include "http_module.h"
/*
* A example for useage of http server
* How to start work?
* ./squick type=tutorial/http
*/
namespace tutorial {
    bool HttpModule::Start() {
        m_element_ = pm_->FindModule<IElementModule>();
        m_class_ = pm_->FindModule<IClassModule>();
        m_http_client_ = pm_->FindModule<IHttpClientModule>();
        m_http_server_ = pm_->FindModule<IHttpServerModule>();
        return true;
    }

    bool HttpModule::AfterStart() {
        m_http_server_->AddRequestHandler("/", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnCommandQuery);
        m_http_server_->AddRequestHandler("/json", HttpType::SQUICK_HTTP_REQ_POST, this, &HttpModule::OnCommandQuery);
        m_http_server_->AddNetFilter("/json", this, &HttpModule::OnFilter);

        m_http_server_->AddRequestHandler("/client/get", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnClientGet);
        m_http_server_->AddRequestHandler("/coro", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnCoroutineQuery);
        m_http_server_->AddRequestHandler("/await", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnCoroutineAsyncRequest);
        // http://127.0.0.1/json
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

    bool HttpModule::BeforeDestory() {
        std::cout << "Hello, world2, BeforeDestory" << std::endl;
        return true;
    }

    bool HttpModule::Destory() {
        std::cout << "Hello, world2, Destory" << std::endl;
        return true;
    }


    Coroutine<bool> HttpModule::OnCoroutineQuery(std::shared_ptr<HttpRequest> req) {
        m_http_server_->ResponseMsg(req, "You query a coroutine function", WebStatus::WEB_OK);
        co_return;
    }

    Coroutine<bool> HttpModule::OnCoroutineAsyncRequest(std::shared_ptr<HttpRequest> req) {
        std::cout << "You use await to async request another server\n";
        auto data = co_await m_http_client_->CoGet("http://www.baidu.com");
        m_http_server_->ResponseMsg(req, data.content, WebStatus::WEB_OK);
        co_return;
    }

    bool HttpModule::OnCommandQuery(std::shared_ptr<HttpRequest> req) {
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
    bool HttpModule::OnClientGet(std::shared_ptr<HttpRequest> req) {
        std::cout << "OnClientGet: request https://bilibili.com \n";
        IHttpServerModule *m = this->m_http_server_;
        m_http_client_->DoGet("https://www.bilibili.com",
            [m, req](const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData) -> void {
                std::cout << "OnClientGet Response:"
                    << "state_code: " << state_code << endl
                    << "content: " << strRespData << endl
                    << "memoData: " << strMemoData << endl;
                m->ResponseMsg(req, strRespData, WebStatus::WEB_OK);
            });
        return true;
    }

    int HttpModule::OnHeartBeat(const Guid& self, const std::string& heartBeat, const float time, const int count) {
        std::cout << heartBeat << std::endl;

        m_http_client_->DoGet("http://127.0.0.1:8080/json", this, &HttpModule::OnGetCallBack);
        m_http_client_->DoGet("http://127.0.0.1:8080/json",
            [](const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData) -> void {
                std::cout << "OnGetCallBack" << std::endl;
            });

        std::string strMemo = "Memo here";
        m_http_client_->DoPost("http://127.0.0.1:8080/json", "OnHeartBeat post data---", this, &HttpModule::OnPostCallBack, strMemo);

        m_http_client_->DoPost("http://127.0.0.1:8080/json", "OnHeartBeat post data---",
            [](const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData) -> void {
                std::cout << "OnPostCallBack" << std::endl;
            });

        return 0;
    }

    void HttpModule::OnGetCallBack(const Guid id, const int state_code, const std::string& strRespData) { std::cout << "OnGetCallBack" << std::endl; }

    void HttpModule::OnPostCallBack(const Guid id, const int state_code, const std::string& strRespData, const std::string& strMemoData) {
        std::cout << "OnPostCallBack"
            << " " << strMemoData << std::endl;
    }
}