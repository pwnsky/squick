

#include "logic_module.h"
#include <squick/plugin/lua/export.h>
// #include <third_party/nlohmann/json.hpp>
namespace player::logic {
bool LogicModule::Start() {
    m_lua_script_ = pm_->FindModule<ILuaScriptModule>();
    m_node_ = pm_->FindModule<INodeModule>();
    m_log_ = pm_->FindModule<ILogModule>();
    vector<int> node_types = {rpc::ST_WORLD, rpc::ST_DB_PROXY, rpc::ST_PLAYER};
    m_node_->AddSubscribeNode(node_types);
    return true;
}

bool LogicModule::Destroy() { return true; }

bool LogicModule::AfterStart() {
    LuaBind();
    return true;
}

bool LogicModule::Update() { return true; }

bool LogicModule::LuaBind() {
    LuaScriptModule *luaModule = dynamic_cast<LuaScriptModule *>(m_lua_script_);
    LuaIntf::LuaContext &luaEnv = luaModule->GetLuaEnv();
    LuaIntf::LuaBinding(luaEnv)
        .beginClass<LogicModule>("CC")
        .addFunction("Test", &LogicModule::Test)
        .endClass();
    // 将该模块传递到lua层
    try {
        LuaIntf::LuaRef func(luaEnv, "CCLogicBind");
        func.call<LuaIntf::LuaRef>(this);
    } catch (LuaIntf::LuaException &e) {
        LOG_ERROR("Call CCLogicBind is error %v", e.what());
    }
    return true;
}

// 发送数据给客户端，用于给player.cc使用
// 由于core/lua 插件中未编译参数表进入，在运行时由 core/lua 在动态链接库内部进行匹配，
// 所以在该动态链接库的函数参数 类型，顺序需要符合 core/lua 中已定义的函数参数顺序
// const Guid player, const uint16_t msg_id, const std::string& data
// 由于是跨dll进行解析，无法对已在core/lua上的Guid进行解析，所以只能传普通类型的数据。

void LogicModule::Test(const uint16_t msg_id, string &msg, int a) { std::cout << "LogicModule::Test\n" << msg_id << "   " << msg << a << std::endl; }

} // namespace player::logic