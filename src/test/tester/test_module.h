#pragma once

#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/config/i_class_module.h>

class ITestModule
	: public IModule
{
};

class TestModule
    : public ITestModule
{
public:
    TestModule( IPluginManager* p )
    {
        m_bIsUpdate = true;
        pPluginManager = p;
    }

    virtual ~TestModule() {};

    virtual bool Awake();
    virtual bool Start();
    virtual bool AfterStart();
    virtual bool CheckConfig();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool BeforeDestory();
    virtual bool Destory();
    virtual bool Finalize();
    virtual bool OnReloadPlugin();


protected:
    int Factorial(int n);

private:
};

