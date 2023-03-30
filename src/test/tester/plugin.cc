#include "plugin.h"
#include "test_module.h"

SQUICK_EXPORT void SquickPluginLoad(IPluginManager *pm){

    CREATE_PLUGIN(pm, TestPlugin)

};

SQUICK_EXPORT void SquickPluginUnload(IPluginManager *pm){DESTROY_PLUGIN(pm, TestPlugin)};

//////////////////////////////////////////////////////////////////////////

const int TestPlugin::GetPluginVersion() { return 0; }

const std::string TestPlugin::GetPluginName() { return GET_CLASS_NAME(TestPlugin); }

void TestPlugin::Install() {
    std::cout << "TestPlugin::Install" << std::endl;
    REGISTER_MODULE(pm_, ITestModule, TestModule)
}

void TestPlugin::Uninstall() { UNREGISTER_MODULE(pm_, ITestModule, TestModule) }