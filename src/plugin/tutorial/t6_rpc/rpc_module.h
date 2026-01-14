#pragma once

#include <core/base.h>
#include <plugin/core/net/export.h>

namespace tutorial {
class ISimpleModule : public IModule {};

class SimpleModule : public ISimpleModule {
  public:
    SimpleModule(IPluginManager *p) {
        is_update_ = true;
        pm_ = p;
    }

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();

    void OnRecivedMsg(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);

  private:
    INetClientModule *m_net_client_;
};

} // namespace tutorial