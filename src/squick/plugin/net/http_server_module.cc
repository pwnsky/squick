
#include "http_server_module.h"
#include "http_server.h"

HttpServerModule::HttpServerModule(IPluginManager *p) {
    pPluginManager = p;
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
    m_pLogModule = pPluginManager->FindModule<ILogModule>();
    m_pHttpServer = new HttpServer(this, &HttpServerModule::OnReceiveNetPack, &HttpServerModule::OnFilterPack);

    return m_pHttpServer->StartServer(nPort);
}

bool HttpServerModule::OnReceiveNetPack(SQUICK_SHARE_PTR<HttpRequest> req) {
    if (req == nullptr) {
        return false;
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
            } catch (const std::exception &) {
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
        m_pLogModule->LogWarning(Guid(), os, __FUNCTION__, __LINE__);
    }

    return ResponseMsg(req, "", WebStatus::WEB_ERROR);
}

WebStatus HttpServerModule::OnFilterPack(SQUICK_SHARE_PTR<HttpRequest> req) {
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
                             SQUICK_SHARE_PTR<std::map<std::string, HTTP_RECEIVE_FUNCTOR_PTR>>(SQUICK_NEW std::map<std::string, HTTP_RECEIVE_FUNCTOR_PTR>()));
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

bool HttpServerModule::ResponseMsg(SQUICK_SHARE_PTR<HttpRequest> req, const std::string &msg, WebStatus code, const std::string &strReason) {
    dout << "\nHttp Response str: " << msg << "\n";
    return m_pHttpServer->ResponseMsg(req, msg, code, strReason);
}