
#include "http_server_module.h"
#include "http_server.h"
#include <squick/core/base.h>

HttpServerModule::HttpServerModule(IPluginManager *p) {
    pm_ = p;
    m_pHttpServer = NULL;
}

HttpServerModule::~HttpServerModule() {
    if (m_pHttpServer) {
        delete m_pHttpServer;
        m_pHttpServer = NULL;
    }
}

bool HttpServerModule::Update() {
    if (m_pHttpServer) {
        m_pHttpServer->Update();
    }
    return true;
}

int HttpServerModule::StartServer(const unsigned short nPort) {
    m_log_ = pm_->FindModule<ILogModule>();
    m_pHttpServer = new HttpServer(this, &HttpServerModule::OnReceiveNetPack, &HttpServerModule::OnFilterPack);

    return m_pHttpServer->StartServer(nPort);
}

bool HttpServerModule::OnReceiveNetPack(std::shared_ptr<HttpRequest> req) {
    if (req == nullptr) {
        return false;
    }
    if (middleware_) {
        auto webstate = middleware_->operator()(req);
        if (webstate != WEB_OK) {
            ResponseMsg(req, "{\"code=-1\", \"msg\"=\"Forbiden\"}", webstate);
            return true;
        }
    }

    Performance performance;
    auto it = mMsgCBMap.GetElement(req->type);
    if (it) {
        auto itPath = it->find(req->path);
        if (it->end() != itPath) {
            HTTP_RECEIVE_FUNCTOR_PTR &pFunPtr = itPath->second;
            HTTP_RECEIVE_FUNCTOR *pFunc = pFunPtr.get();
            try {
                pFunc->operator()(req);
            } catch (const std::exception &e) {
                dout << "Http quest error: " << e.what() << std::endl;
                ResponseMsg(req, "unknow error", WebStatus::WEB_INTER_ERROR);
            }
            return true;
        }
    }

    if (performance.CheckTimePoint(1)) {
        std::ostringstream os;
        os << "---------------net module performance problem------------------- ";
        os << performance.TimeScope();
        os << "---------- ";
        os << req->path;
        m_log_->LogWarning(Guid(), os, __FUNCTION__, __LINE__);
    }

    return ResponseMsg(req, "", WebStatus::WEB_ERROR);
}

WebStatus HttpServerModule::OnFilterPack(std::shared_ptr<HttpRequest> req) {
    if (req == nullptr) {
        return WebStatus::WEB_INTER_ERROR;
    }

    auto itPath = mMsgFliterMap.find(req->path);
    if (mMsgFliterMap.end() != itPath) {
        HTTP_FILTER_FUNCTOR_PTR &pFunPtr = itPath->second;
        HTTP_FILTER_FUNCTOR *pFunc = pFunPtr.get();
        return pFunc->operator()(req);
    }

    return WebStatus::WEB_OK;
}

bool HttpServerModule::AddMsgCB(const std::string &strCommand, const HttpType eRequestType, const HTTP_RECEIVE_FUNCTOR_PTR &cb) {
    auto it = mMsgCBMap.GetElement(eRequestType);
    if (!it) {
        mMsgCBMap.AddElement(eRequestType,
                             std::shared_ptr<std::map<std::string, HTTP_RECEIVE_FUNCTOR_PTR>>(new std::map<std::string, HTTP_RECEIVE_FUNCTOR_PTR>()));
    }

    it = mMsgCBMap.GetElement(eRequestType);
    auto itPath = it->find(strCommand);
    if (it->end() == itPath) {
        it->insert(std::map<std::string, HTTP_RECEIVE_FUNCTOR_PTR>::value_type(strCommand, cb));

        return true;
    } else {
        std::cout << eRequestType << " " << strCommand << "" << std::endl;
    }

    return false;
}

bool HttpServerModule::AddFilterCB(const std::string &strCommand, const HTTP_FILTER_FUNCTOR_PTR &cb) {
    auto it = mMsgFliterMap.find(strCommand);
    if (it == mMsgFliterMap.end()) {
        mMsgFliterMap.insert(std::map<std::string, HTTP_FILTER_FUNCTOR_PTR>::value_type(strCommand, cb));
    }
    return true;
}

bool HttpServerModule::AddMiddlewareCB(const HTTP_FILTER_FUNCTOR_PTR& cb) {
    if (middleware_ == nullptr) {
        middleware_ = cb;
        return true;
    }
    return false;
}

bool HttpServerModule::ResponseMsg(std::shared_ptr<HttpRequest> req, const std::string &msg, WebStatus code, const std::string &strReason) {
    dout << "\nHttp Response str: " << msg << "\n";
    return m_pHttpServer->ResponseMsg(req, msg, code, strReason);
}

bool HttpServerModule::SetHeader(std::shared_ptr<HttpRequest> req, const std::string& key, const std::string& value) {
    return m_pHttpServer->SetHeader(req, key, value);
}