#pragma once

#include <squick/core/base.h>
#include <squick/plugin/net/export.h>

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

    void OnRecivedMsg(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);

private:
    INetClientModule* m_net_client_;
};



}