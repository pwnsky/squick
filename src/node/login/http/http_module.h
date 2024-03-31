#pragma once

#include <iostream>
#include <map>
#include "i_http_module.h"
#include <squick/core/platform.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>
#include <third_party/nlohmann/json.hpp>
#include <node/login/node/i_node_module.h>
#include <map>
#include <unordered_map>
namespace login::http {
class HttpModule : public IHttpModule {
    using json = nlohmann::json;

  public:
    HttpModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool AfterStart();
    virtual bool Update();

  protected:
    Coroutine<bool>  OnLogin(std::shared_ptr<HttpRequest> request);
    WebStatus Middleware(std::shared_ptr<HttpRequest> request);
    nlohmann::json GetUser(std::shared_ptr<HttpRequest> request);
    bool CheckAuth(const std::string &guid, const std::string &token);
    void PrintRequest(std::shared_ptr<HttpRequest> req);

    void SetToken(const std::string& account_id, const std::string& user_token);

  private:
    string MakeToken(string sguid);
    Guid CreatePlayerGUID();
    INetClientModule *m_net_client_;
    IHttpServerModule *m_http_server_;
    node::INodeModule *m_node_;
private:
    std::map<std::string, std::string> auth_token_;

  private:
    int player_index = 0;
};

} // namespace login::http