#pragma once

#include <iostream>
#include <map>

#include "../client/i_master_module.h"
#include "../server/i_server_module.h"
#include "i_http_module.h"
#include <squick/core/platform.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/net/export.h>
#include <squick/struct/struct.h>
#include <third_party/nlohmann/json.hpp>

#include <server/login/redis/i_redis_module.h>
#include <server/login/mysql/i_mysql_module.h>

#include <unordered_map>
namespace login::http {
class HttpModule : public IHttpModule {
    using json = nlohmann::json;
  public:
    HttpModule(IPluginManager *p) {
        pPluginManager = p;
        m_bIsUpdate = true;
    }

    virtual bool Start();
    virtual bool Destory();
    virtual bool AfterStart();
    virtual bool Update();

  protected:
    bool OnLogin(SQUICK_SHARE_PTR<HttpRequest> request);
    bool OnWorldList(SQUICK_SHARE_PTR<HttpRequest> request);
    bool OnWorldEnter(SQUICK_SHARE_PTR<HttpRequest> request);
    WebStatus OnFilter(SQUICK_SHARE_PTR<HttpRequest> request);
    std::string GetUserID(SQUICK_SHARE_PTR<HttpRequest> request);
    std::string GetUserJWT(SQUICK_SHARE_PTR<HttpRequest> request);
    bool CheckUserJWT(const std::string &user, const std::string &jwt);
    std::unordered_map<std::string, std::string> tokens_;
    
    bool OnGetCDN(SQUICK_SHARE_PTR<HttpRequest> request);
  private:
    INetClientModule *m_net_client_;
    IKernelModule *m_kernel_;
    IHttpServerModule *m_http_server_;
    server::IServerModule *m_server_;
    client::IMasterModule *m_master_;
    IClassModule *m_class_;
    IElementModule *m_element_;
    redis::IRedisModule *m_redis_;
    mysql::IMysqlModule* m_mysql_;
};

} // namespace login::http