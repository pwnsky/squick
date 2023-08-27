

#include "lua_bind_module.h"
#include <squick/plugin/lua/export.h>
// #include <third_party/nlohmann/json.hpp>
namespace lobby::lua {
bool LuaBindModule::Start() {
    m_lua_script_ = pm_->FindModule<ILuaScriptModule>();
    m_player_manager_ = pm_->FindModule<player::IPlayerManagerModule>();
    m_node_ = pm_->FindModule<node::INodeModule>();
    Bind();
    return true;
}

bool LuaBindModule::Destory() { return true; }

bool LuaBindModule::AfterStart() { return true; }

bool LuaBindModule::Update() { return true; }

bool LuaBindModule::Bind() {
    LuaScriptModule *luaModule = dynamic_cast<LuaScriptModule *>(m_lua_script_);
    LuaIntf::LuaContext &luaEnv = luaModule->GetLuaEnv();
    // SendMsgToGate(const uint16_t msg_id, const std::string& msg, const Guid& self)
    LuaIntf::LuaBinding(luaEnv)
        .beginClass<LuaBindModule>("GameServer")
        .addFunction("send_to_player", &LuaBindModule::SendToPlayer)
        .addFunction("test", &LuaBindModule::Test)
        .endClass();
    // 将该模块传递到lua层
    try {
        LuaIntf::LuaRef func(luaEnv, "init_game_server");
        func.call<LuaIntf::LuaRef>(this);
    } catch (LuaIntf::LuaException &e) {
        cout << e.what() << endl;
    }
    return true;
}

// 发送数据给客户端，用于给player.cc使用
// 由于core/lua 插件中未编译参数表进入，在运行时由 core/lua 在动态链接库内部进行匹配，
// 所以在该动态链接库的函数参数 类型，顺序需要符合 core/lua 中已定义的函数参数顺序
// const Guid player, const uint16_t msg_id, const std::string& data
// 由于是跨dll进行解析，无法对已在core/lua上的Guid进行解析，所以只能传普通类型的数据。
void LuaBindModule::SendToPlayer(string &player_guid_str, uint16_t msg_id, std::string &data) {
    std::cout << "this: " << this << "  " << m_node_ << " LuaBindModule::SendToPlayer " << msg_id << " msg: " << data << std::endl;
    // std::cout << " \n length: " << data.length() << std::endl;
    Guid guid = Guid(player_guid_str);
    //m_node_->SendToPlayer(msg_id, data, guid);
}

void LuaBindModule::Test(const uint16_t msg_id, string &msg, int a) { std::cout << "LuaBindModule::Test\n" << msg_id << "   " << msg << a << std::endl; }

} // namespace lobby::lua