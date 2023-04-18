#pragma once

#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/struct/struct.h>

#include "i_server_module.h"
namespace master::server {
class ServerModule : public IServerModule {
  public:
    ServerModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }
    virtual ~ServerModule();

    virtual bool Start();
    virtual bool Destory();

    virtual bool AfterStart();
    virtual bool Update();

    virtual void LogReceive(const char *str) {}
    virtual void LogSend(const char *str) {}
    virtual std::string GetServersStatus();

  protected:
    void OnClientDisconnect(const socket_t sock);
    void OnClientConnected(const socket_t sock);

  protected:
    void OnServerReport(const socket_t nFd, const int msg_id, const char *buffer, const uint32_t len);
    void OnHeartBeat(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
    IElementModule *m_element_;
    IClassModule *m_class_;
    IKernelModule *m_kernel_;
    ILogModule *m_log_;
    INetModule *m_net_;
};

} // namespace master::server