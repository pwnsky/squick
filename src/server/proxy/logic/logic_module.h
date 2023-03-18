#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_client_module.h>
#include <squick/plugin/net/i_net_module.h>

#include "i_logic_module.h"
class ProxyLogicModule : public IProxyLogicModule {
  public:
    ProxyLogicModule(IPluginManager *p) { pPluginManager = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

  protected:
    void OnHeartbeat(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);

  protected:
    IClassModule *m_pClassModule;
    IKernelModule *m_pKernelModule;
    INetModule *m_pNetModule;
    INetClientModule *m_pNetClientModule;

  private:
};
