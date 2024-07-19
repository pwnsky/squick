#pragma once
#include "i_logic_module.h"
#include <iostream>
#include <map>
#include <set>
#include <squick/core/platform.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/node/export.h>
#include <struct/struct.h>
#include <third_party/nlohmann/json.hpp>


namespace backstage::logic {
struct LoginInfo {
    std::string account;
    std::string account_id;
    int proxy_node;
    time_t login_time;
    int32_t limit_time;
};
class LogicModule : public ILogicModule {
    using json = nlohmann::json;

  public:
    LogicModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }

    virtual bool Start();
    virtual bool Destroy();
    virtual bool AfterStart();
    virtual bool Update();

  protected:
    Coroutine<bool> OnLogin(std::shared_ptr<HttpRequest> request);
    WebStatus Middleware(std::shared_ptr<HttpRequest> request);
    json GetUser(std::shared_ptr<HttpRequest> request);
    bool CheckAuth(const std::string &guid, const std::string &token);

    void SetToken(const std::string &account_id, const std::string &user_token);
    bool LoadConfig();
    bool OnAuthCheck(std::shared_ptr<HttpRequest> request);
    Coroutine<bool> OnGetAllNodes(std::shared_ptr<HttpRequest> request);

  private:
    string MakeToken(string sguid);
    INetClientModule *m_net_client_;
    INetModule *m_net_;
    IHttpServerModule *m_http_server_;
    INodeModule *m_node_;
    ILogModule *m_log_;

  private:
    std::map<std::string, std::string> auth_token_;
    std::map<std::string, LoginInfo> login_info_;
    std::set<std::string> white_uri_list_;
    std::map<std::string, std::string> config_response_header_;

  private:
    int player_index = 0;
    json web_config_;
};

} // namespace backstage::logic