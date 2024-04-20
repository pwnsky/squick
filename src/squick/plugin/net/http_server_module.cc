
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

    time_t now_time = time(nullptr);
    if (now_time - last_check_coroutines_time_ > 0) {
        FixCoroutines(now_time);
        last_check_coroutines_time_ = now_time;
    }
    return true;
}

int HttpServerModule::FixCoroutines(time_t now_time) {
    // Check coroutine states and free coroutines objects
    int num = 0;
    for (auto iter = coroutines_.begin(); iter != coroutines_.end();) {
        auto now_iter = iter;
        ++iter;
        auto co = *now_iter;
        if (co.GetHandle().done()) {
            co.GetHandle().destroy();
#ifdef SQUICK_DEV
            dout << "Destoy coroutine: " << co.GetHandle().address() << endl;
#endif
            coroutines_.erase(now_iter);
            num++;
            continue;
        }

        if (now_time - co.GetStartTime() > HTTP_SERVER_COROTINE_MAX_SURVIVAL_TIME) {
#ifdef SQUICK_DEV
            dout << " This corotine has time out: " << co.GetHandle().address() << std::endl;
#endif
            // do not destroy
            if (!co.GetHandle().done()) {
                co.GetHandle().resume();
            }
        }
    }

    return num;
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
            return false;
        }
    }

    Performance performance;
    // first find coro funcs
    auto co_funcs = coro_funcs_.GetElement(req->type);
    if (co_funcs) {
        auto co_iter = co_funcs->find(req->path);
        if (co_funcs->end() != co_iter) {
            HTTP_RECEIVE_CORO_FUNCTOR_PTR &pFunPtr = co_iter->second;
            HTTP_RECEIVE_CORO_FUNCTOR *pFunc = pFunPtr.get();
            try {
                // create a coroutine...

                auto co = pFunc->operator()(req);
                coroutines_.push_back(co);
#ifdef SQUICK_DEV
                dout << "Create a new coroutine: " << co.GetHandle().address() << endl;
#endif
                // to run this coroutine
                co.GetHandle().resume();
            } catch (const std::exception &e) {
                dout << "Http quest error: " << e.what() << std::endl;
                ResponseMsg(req, "unknow error", WebStatus::WEB_INTER_ERROR);
            }
            return true;
        }
    }

    // second find callback funcs
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
        LOG_WARN("http server handle time is high, tiimescop<%v>, req_path<%v>", performance.TimeScope(), req->path);
    }
    SetSquickMainThreadSleep(false);
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
bool HttpServerModule::AddMsgCB(const std::string &strCommand, const HttpType eRequestType, const HTTP_RECEIVE_CORO_FUNCTOR_PTR &cb) {
    auto it = coro_funcs_.GetElement(eRequestType);
    if (!it) {
        coro_funcs_.AddElement(
            eRequestType, std::shared_ptr<std::map<std::string, HTTP_RECEIVE_CORO_FUNCTOR_PTR>>(new std::map<std::string, HTTP_RECEIVE_CORO_FUNCTOR_PTR>()));
    }

    it = coro_funcs_.GetElement(eRequestType);
    auto itPath = it->find(strCommand);
    if (it->end() == itPath) {
        it->insert(std::map<std::string, HTTP_RECEIVE_CORO_FUNCTOR_PTR>::value_type(strCommand, cb));

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

bool HttpServerModule::AddMiddlewareCB(const HTTP_FILTER_FUNCTOR_PTR &cb) {
    if (middleware_ == nullptr) {
        middleware_ = cb;
        return true;
    }
    return false;
}

bool HttpServerModule::ResponseMsg(std::shared_ptr<HttpRequest> req, const std::string &msg, WebStatus code, const std::string &strReason) {
    return m_pHttpServer->ResponseMsg(req, msg, code, strReason);
}

bool HttpServerModule::SetHeader(std::shared_ptr<HttpRequest> req, const std::string &key, const std::string &value) {
    return m_pHttpServer->SetHeader(req, key, value);
}