#pragma once

#include <iostream>
#include <map>
#include "i_http_module.h"
#include <squick/core/platform.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>
#include <third_party/nlohmann/json.hpp>

#include <node/login/mysql/i_mysql_module.h>
#include <node/login/redis/i_redis_module.h>
#include <node/login/node/i_node_module.h>

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
    bool OnLogin(std::shared_ptr<HttpRequest> request);
    bool OnWorldList(std::shared_ptr<HttpRequest> request);
    bool OnWorldEnter(std::shared_ptr<HttpRequest> request);
    WebStatus Middleware(std::shared_ptr<HttpRequest> request);
    std::string GetCookie(std::shared_ptr<HttpRequest> request);
    bool CheckUserJWT(const std::string &user, const std::string &jwt);
    

    bool OnGetCDN(std::shared_ptr<HttpRequest> request);
    void PrintRequest(std::shared_ptr<HttpRequest> req);

  private:
    string MakeToken(string sguid);
    INetClientModule *m_net_client_;
    IKernelModule *m_kernel_;
    IHttpServerModule *m_http_server_;
    node::INodeModule *m_node_;
    IClassModule *m_class_;
    IElementModule *m_element_;
    redis::IRedisModule *m_redis_;
    mysql::IMysqlModule *m_mysql_;
};

} // namespace login::http