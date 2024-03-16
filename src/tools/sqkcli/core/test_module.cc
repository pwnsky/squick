

#include "test_module.h"

bool TestModule::Awake() {
    int argc = 0;
    char *c = new char[1];
    //::testing::GTEST_FLAG(output) = "xml:hello.xml";
    //::testing::StartGoogleTest(&argc, &c);
    std::cout << "我已经运行了" << std::endl;
    std::list<IModule *> xModules = pm_->Modules(); // 获取全部模块，
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name);
        //std::cout << "尝试查找：" << pModule->name << std::endl;
        if (nullptr != pTestModule) {
            pTestModule->Awake();
        } else {
            //std::cout << "查找模块出错，为空" << std::endl;
        }
    }

    return true;
}

bool TestModule::Start() {
    // find all plugins and all modules, then check whether they have a tester
    // if any module have't a tester for it then  can not start the application
    // this is a rule for Squick's world to keep high quality code under TDD

    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name);
        //pTestModule->Start();
    }

    return true;
}

bool TestModule::AfterStart() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name);
        pTestModule->AfterStart();
    }

    return true;
}

bool TestModule::CheckConfig() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name);
        pTestModule->CheckConfig();
    }

    return true;
}

bool TestModule::ReadyUpdate() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name);
        pTestModule->ReadyUpdate();
    }

    return true;
}

bool TestModule::Update() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name);
        pTestModule->Update();
    }

    return true;
}

bool TestModule::BeforeDestory() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name);
        pTestModule->BeforeDestory();
    }

    return true;
}

bool TestModule::Destory() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name);
        pTestModule->Destory();
    }

    return true;
}

bool TestModule::Finalize() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name);
        pTestModule->Finalize();
    }

    return true;
}

bool TestModule::OnReloadPlugin() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name);
        pTestModule->OnReloadPlugin();
    }

    return true;
}

int TestModule::Factorial(int n) {
    if (1 == n) {
        return 1;
    }

    return n * Factorial(n - 1);
}

// 测试用例
/*
TEST(FactorialTest, HandlesPositiveInput) {
    EXPECT_EQ(1, this->Factorial(1));
    EXPECT_EQ(2, this->Factorial(2));
    EXPECT_EQ(6, this->Factorial(3));
    EXPECT_EQ(40320, this->Factorial(8));
}
*/