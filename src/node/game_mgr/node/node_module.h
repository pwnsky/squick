#pragma once

#include "i_node_module.h"
#include <map>

namespace gameplay_manager::node {

class NodeModule : public INodeModule {
  public:
    NodeModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }

    virtual bool Destroy();

    virtual bool AfterStart();

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

} // namespace gameplay_manager::node
