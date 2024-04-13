

#include "test_module.h"

bool TestModule::Awake() {
    int argc = 0;
    char *c = new char[1];
    //::testing::GTEST_FLAG(output) = "xml:hello.xml";
    //::testing::StartGoogleTest(&argc, &c);
    std::cout << "Running" << std::endl;
    std::list<IModule *> xModules = pm_->Modules(); // Get all moduels
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name_);
        if (nullptr != pTestModule) {
            pTestModule->Awake();
        } else {
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
        IModule *pTestModule = pm_->FindTestModule(pModule->name_);
        //pTestModule->Start();
    }

    return true;
}

bool TestModule::AfterStart() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name_);
        pTestModule->AfterStart();
    }

    return true;
}

bool TestModule::CheckConfig() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name_);
        pTestModule->CheckConfig();
    }

    return true;
}

bool TestModule::ReadyUpdate() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name_);
        pTestModule->ReadyUpdate();
    }

    return true;
}

bool TestModule::Update() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name_);
        pTestModule->Update();
    }

    return true;
}

bool TestModule::BeforeDestroy() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name_);
        pTestModule->BeforeDestroy();
    }

    return true;
}

bool TestModule::Destroy() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name_);
        pTestModule->Destroy();
    }

    return true;
}

bool TestModule::Finalize() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name_);
        pTestModule->Finalize();
    }

    return true;
}

bool TestModule::OnReloadPlugin() {
    std::list<IModule *> xModules = pm_->Modules();
    for (auto it : xModules) {
        IModule *pModule = it;
        IModule *pTestModule = pm_->FindTestModule(pModule->name_);
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

// Test
/*
TEST(FactorialTest, HandlesPositiveInput) {
    EXPECT_EQ(1, this->Factorial(1));
    EXPECT_EQ(2, this->Factorial(2));
    EXPECT_EQ(6, this->Factorial(3));
    EXPECT_EQ(40320, this->Factorial(8));
}
*/