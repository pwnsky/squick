#pragma once

#include <core/map.h>
#include <plugin/core/config/export.h>
#include <plugin/core/log/export.h>
#include <plugin/core/net/export.h>

#include "i_logic_module.h"

namespace gameplay_manager::logic {
class LogicModule : public ILogicModule {
  public:
    LogicModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destroy();
    virtual bool Update();

    virtual bool AfterStart();

  protected:
    void OnLagTestProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    void OnReqPvpInstanceCreate(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    int GetUnbindPort();

  protected:
    IClassModule *m_class_;
    IElementModule *m_element_;
    INetModule *m_net_;
    INetClientModule *m_net_client_;

  private:
    std::string public_ip_ = "";
    int public_port_ = 0;
};

} // namespace gameplay_manager::logic