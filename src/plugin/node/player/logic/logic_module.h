#pragma once

#include <iostream>
#include <map>

#include <core/platform.h>
#include <plugin/core/config/export.h>
#include <plugin/core/lua/export.h>
#include <plugin/core/net/export.h>
#include <plugin/core/node/export.h>
#include <plugin/core/log/export.h>
#include <struct/struct.h>

namespace player::logic {
class ILogicModule : public IModule {};

class LogicModule : public ILogicModule {
  public:
    LogicModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = false;
    }

    virtual bool Start();
    virtual bool Destroy();

    virtual bool AfterStart();
    virtual bool Update();

  protected:
    bool LuaBind();
    void Test(const uint16_t msg_id, string &msg, int a);

  private:
    ILuaScriptModule *m_lua_script_;
    INodeModule *m_node_;
    ILogModule *m_log_;
};

} // namespace player::logic
