#pragma once

#include <squick/core/map.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_logic_module.h"

namespace gameplay_manager::logic {
class LogicModule : public ILogicModule {
  public:
    LogicModule(IPluginManager *p) { pPluginManager = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

  protected:
    void OnLagTestProcess(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    void OnReqPvpInstanceCreate(const SQUICK_SOCKET sockIndex, const int msgID, const char *msg, const uint32_t len);
    int GetUnbindPort();

  protected:
    IClassModule * m_class_;
    IElementModule* m_element_;
    IKernelModule *m_pKernelModule;
    INetModule *m_pNetModule;
    INetClientModule *m_pNetClientModule;

  private:
      std::string public_ip_ = "";
      int public_port_ = 0;
};

} // namespace gameplay_manager::logic