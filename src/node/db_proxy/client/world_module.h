#ifndef SQUICK_DB_NET_CLIENT_MODULE_H
#define SQUICK_DB_NET_CLIENT_MODULE_H

#include "squick/struct/struct.h"

#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>

#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/security/i_security_module.h>

#include "i_world_module.h"
#include <server/world/client/i_master_module.h>
#include <server/world/server/i_server_module.h>

class DBToWorldModule : public IDBToWorldModule {
  public:
    DBToWorldModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }

    virtual bool Start();
    virtual bool BeforeDestory();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

  protected:
    void OnClientDisconnect(const socket_t sock);

    void OnClientConnected(const socket_t sock);

    void InvalidMessage(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

  private:
};

#endif