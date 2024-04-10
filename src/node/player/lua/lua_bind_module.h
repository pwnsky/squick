#pragma once

#include <iostream>
#include <map>

#include <squick/core/platform.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/lua/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>

namespace player::lua {
class ILuaBindModule : public IModule {};

class LuaBindModule : public ILuaBindModule {
  public:
    LuaBindModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = false;
    }

    virtual bool Start();
    virtual bool Destroy();

    virtual bool AfterStart();
    virtual bool Update();

  protected:
    bool Bind();

    void SendToPlayer(string &player_guid_str, uint16_t msg_id, std::string &data);
    void Test(const uint16_t msg_id, string &msg, int a);

  private:
    ILuaScriptModule *m_lua_script_;
    //player::IPlayerManagerModule *m_player_manager_;
    //node::INodeModule *m_node_;
};

} // namespace player::lua
