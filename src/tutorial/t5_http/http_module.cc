
#include "http_module.h"

bool HelloWorld5::Start() {
    m_element_ = pm_->FindModule<IElementModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_http_client_ = pm_->FindModule<IHttpClientModule>();
    m_http_server_ = pm_->FindModule<IHttpServerModule>();
    return true;
}

bool HelloWorld5::AfterStart() {
    std::cout << "Hello, world, Start" << std::endl;
    // http://127.0.0.1/json
    m_http_server_->AddRequestHandler("/json", HttpType::SQUICK_HTTP_REQ_GET, this, &HelloWorld5::OnCommandQuery);
    m_http_server_->AddRequestHandler("/json", HttpType::SQUICK_HTTP_REQ_POST, this, &HelloWorld5::OnCommandQuery);
    m_http_server_->AddNetFilter("/json", this, &HelloWorld5::OnFilter);
    m_http_server_->StartServer(8080);
    return true;
}

bool HelloWorld5::Update() {
    if (m_http_server_)
        m_http_server_->Update();
    return true;
}

bool HelloWorld5::BeforeDestory() {
    std::cout << "Hello, world2, BeforeDestory" << std::endl;
    return true;
}

bool HelloWorld5::Destory() {
    std::cout << "Hello, world2, Destory" << std::endl;
    return true;
}

bool HelloWorld5::OnCommandQuery(std::shared_ptr<HttpRequest> req) {
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

    return m_http_server_->ResponseMsg(req, "OnCommandQuery --- test1", WebStatus::WEB_OK);
}

WebStatus HelloWorld5::OnFilter(std::shared_ptr<HttpRequest> req) {
    std::cout << "OnFilter ... " << std::endl;

    return WebStatus::WEB_OK;
}

int HelloWorld5::OnHeartBeat(const Guid &self, const std::string &heartBeat, const float time, const int count) {
    std::cout << heartBeat << std::endl;

    m_http_client_->DoGet("http://127.0.0.1:8080/json", this, &HelloWorld5::OnGetCallBack);
    m_http_client_->DoGet("http://127.0.0.1:8080/json",
                               [](const Guid id, const int state_code, const std::string &strRespData, const std::string &strMemoData) -> void {
                                   std::cout << "OnGetCallBack" << std::endl;
                               });

    std::string strMemo = "Memo here";
    m_http_client_->DoPost("http://127.0.0.1:8080/json", "OnHeartBeat post data---", this, &HelloWorld5::OnPostCallBack, strMemo);

    m_http_client_->DoPost("http://127.0.0.1:8080/json", "OnHeartBeat post data---",
                                [](const Guid id, const int state_code, const std::string &strRespData, const std::string &strMemoData) -> void {
                                    std::cout << "OnPostCallBack" << std::endl;
                                });

    return 0;
}

void HelloWorld5::OnGetCallBack(const Guid id, const int state_code, const std::string &strRespData) { std::cout << "OnGetCallBack" << std::endl; }

void HelloWorld5::OnPostCallBack(const Guid id, const int state_code, const std::string &strRespData, const std::string &strMemoData) {
    std::cout << "OnPostCallBack"
              << " " << strMemoData << std::endl;
}