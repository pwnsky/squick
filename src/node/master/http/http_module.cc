#include "http_module.h"
#include <third_party/nlohmann/json.hpp>

using json = nlohmann::json;

namespace master::http {
bool HttpModule::Start() {
    m_http_server_ = pm_->FindModule<::IHttpServerModule>();
    m_node_ = pm_->FindModule<node::INodeModule>();
    return true;
}
bool HttpModule::Destroy() { return true; }

bool HttpModule::AfterStart() {
    // http://127.0.0.1/json
    m_http_server_->AddRequestHandler("/node/list", HttpType::SQUICK_HTTP_REQ_GET, this, &HttpModule::OnGetNodeList);

    m_http_server_->StartServer(pm_->GetArg("http_port = ", 8888));
    return true;
}

bool HttpModule::Update() {
    m_http_server_->Update();
    return true;
}

bool HttpModule::OnGetNodeList(std::shared_ptr<HttpRequest> req) {

    auto &nodes = m_node_->GetAllNodes();

    json statusRoot;

    statusRoot["code"] = 0;
    statusRoot["msg"] = "";
    statusRoot["time"] = pm_->GetNowTime();
    for (auto& s : nodes) {
        auto& sd = s.second.info;
        json n;
        n["area"] = sd->area();
        n["type"] = sd->type();
        n["id"] = sd->id();
        n["name"] = sd->name().c_str();
        n["ip"] = sd->ip().c_str();
        n["public_ip"] = sd->public_ip().c_str();
        n["port"] = sd->port();
        n["cpu_count"] = sd->cpu_count();
        n["status"] = sd->state();
        n["workload"] = sd->workload();
        n["max_online"] = sd->max_online();
        n["update_time"] = sd->update_time();
        n["ws_port"] = sd->ws_port();
        n["http_port"] = sd->http_port();
        n["https_port"] = sd->https_port();

        statusRoot["node_list"][to_string(sd->id())] = n;
    }
    return m_http_server_->ResponseMsg(req, statusRoot.dump(), WebStatus::WEB_OK);
}

} // namespace master::http_server