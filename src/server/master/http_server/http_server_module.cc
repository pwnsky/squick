

#include "http_server_module.h"

bool MasterNet_HttpServerModule::Start() {
    m_http_server_ = pm_->FindModule<IHttpServerModule>();
    m_kernel_ = pm_->FindModule<IKernelModule>();
    m_pMasterServerModule = pm_->FindModule<IMasterNet_ServerModule>();
    m_class_ = pm_->FindModule<IClassModule>();
    m_element_ = pm_->FindModule<IElementModule>();

    return true;
}
bool MasterNet_HttpServerModule::Destory() { return true; }

bool MasterNet_HttpServerModule::AfterStart() {
    // http://127.0.0.1/json
    m_http_server_->AddRequestHandler("/status", HttpType::SQUICK_HTTP_REQ_GET, this, &MasterNet_HttpServerModule::OnCommandQuery);

    m_http_server_->AddNetFilter("/status", this, &MasterNet_HttpServerModule::OnFilter);

    std::shared_ptr<IClass> xLogicClass = m_class_->GetElement(excel::Server::ThisName());
    if (xLogicClass) {
        const std::vector<std::string> &strIdList = xLogicClass->GetIDList();
        for (int i = 0; i < strIdList.size(); ++i) {
            const std::string &strId = strIdList[i];

            int nJsonPort = m_element_->GetPropertyInt32(strId, excel::Server::WebPort());
            int nWebServerAppID = m_element_->GetPropertyInt32(strId, excel::Server::ServerID());

            // webserver only run one instance in each server
            if (pm_->GetAppID() == nWebServerAppID) {
                m_http_server_->StartServer(nJsonPort);

                break;
            }
        }
    }

    return true;
}

bool MasterNet_HttpServerModule::Update() {
    // std::cout << "ookkkkk" << std::endl;
    m_http_server_->Update();
    return true;
}

bool MasterNet_HttpServerModule::OnCommandQuery(std::shared_ptr<HttpRequest> req) {

    std::cout << "url: " << req->url << std::endl;
    std::cout << "path: " << req->path << std::endl;
    std::cout << "type: " << req->type << std::endl;
    std::cout << "body: " << req->body << std::endl;
    /*
    std::cout << "params: " << std::endl;

    for (auto item : req->params)
    {
            std::cout << item.first << ":" << item.second << std::endl;
    }

  std::cout << "headers: " << std::endl;

    for (auto item : req->headers)
    {
            std::cout << item.first << ":" << item.second << std::endl;
    }*/

    std::string str = m_pMasterServerModule->GetServersStatus();
    return m_http_server_->ResponseMsg(req, str, WebStatus::WEB_OK);
}

WebStatus MasterNet_HttpServerModule::OnFilter(std::shared_ptr<HttpRequest> req) {
    /*
    std::cout << "OnFilter: " << std::endl;

    std::cout << "url: " << req->url << std::endl;
    std::cout << "path: " << req->path << std::endl;
    std::cout << "type: " << req->type << std::endl;
    std::cout << "body: " << req->body << std::endl;

    std::cout << "params: " << std::endl;

    for (auto item : req->params)
    {
            std::cout << item.first << ":" << item.second << std::endl;
    }

    std::cout << "headers: " << std::endl;

    for (auto item : req->headers)
    {
            std::cout << item.first << ":" << item.second << std::endl;
    }*/

    return WebStatus::WEB_OK;
}
