#pragma once

#include <iostream>
#include <map>

#include <squick/core/platform.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/lua/export.h>
#include <squick/plugin/net/export.h>
#include <squick/struct/struct.h>

namespace gateway::server {
class IServerModule : public IModule {};

class ServerModule : public IServerModule {
  public:
    ServerModule(IPluginManager *p) {
        pPluginManager = p;
        m_bIsUpdate = true;
    }

    virtual bool Start();
    virtual bool Destory();

    virtual bool AfterStart();
    virtual bool Update();

  protected:
    bool OnGetServerList(SQUICK_SHARE_PTR<HttpRequest> req);

  private:
    IKernelModule *m_pKernelModule;
    IHttpServerModule *m_pHttpNetModule;
    IClassModule *m_pLogicClassModule;
    IElementModule *m_pElementModule;
    ILuaScriptModule *m_pLuaScriptModule;
};

} // namespace gateway::server
