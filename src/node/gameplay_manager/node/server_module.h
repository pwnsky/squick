#pragma once

#include "i_server_module.h"
#include "squick/core/map.h"
#include <iostream>
#include <map>
#include <squick/core/platform.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/lua/export.h>
#include <squick/plugin/net/export.h>
#include <squick/plugin/security/export.h>
#include <squick/struct/struct.h>

namespace gameplay_manager::server {

class ServerModule : public IServerModule {
  public:
    ServerModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }

    virtual bool Start();
    virtual bool Destory();

    virtual bool AfterStart();
    virtual bool Update();

    virtual bool Transport(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  protected:
    // PVP服务器连接处理
    void OnReqConnect(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnOtherMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    // PVP服务器断开连接
    void OnClientDisconnect(const socket_t sock);
    // 转发消息给PVP服务器

  protected:
};

} // namespace gameplay_manager::server
