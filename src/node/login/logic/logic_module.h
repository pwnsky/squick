#pragma once
#include "i_logic_module.h"
#include <iostream>
#include <map>
#include <squick/core/platform.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>
#include <third_party/nlohmann/json.hpp>
#include <node/login/node/i_node_module.h>
#include <map>
#include <unordered_map>

struct LoginInfo {
    std::string account;
    std::string account_id;
    int proxy_node;
    time_t login_time;
    int32_t limit_time;
    int proxy_port;
    int proxy_ws_port;
    std::string proxy_key;
    uint64_t signatrue;
};

namespace login::logic {
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
    Coroutine<bool>  OnLogin(std::shared_ptr<HttpRequest> request);
    WebStatus Middleware(std::shared_ptr<HttpRequest> request);
    nlohmann::json GetUser(std::shared_ptr<HttpRequest> request);
    bool CheckAuth(const std::string &guid, const std::string &token);

    void SetToken(const std::string& account_id, const std::string& user_token);
    virtual void OnConnectProxyVerify(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    string MakeToken(string sguid);
    INetClientModule *m_net_client_;
    INetModule *m_net_;
    IHttpServerModule *m_http_server_;
    node::INodeModule *m_node_;
    ILogModule *m_log_;
private:
    std::map<std::string, std::string> auth_token_;
    std::map<std::string, LoginInfo> login_info_;

  private:
    int player_index = 0;
};

} // namespace login::http