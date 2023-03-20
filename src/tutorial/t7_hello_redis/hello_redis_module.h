#pragma once

#include "squick/core/performance.h"
#include "squick/plugin/no_sql/redis_client.h"
#include <squick/core/base.h>
#include <squick/plugin/navigation/export.h>
#include <squick/plugin/no_sql/export.h>

class IHelloWorld7 : public IModule {};

class HelloWorld7 : public IHelloWorld7 {
  public:
    HelloWorld7(IPluginManager *p) {
        m_bIsUpdate = true;
        pPluginManager = p;
    }

    virtual bool Start();
    virtual bool AfterStart();

    virtual bool Update();

    virtual bool BeforeDestory();
    virtual bool Destory();

  protected:
    bool Test_1();
    bool TestPerformance();

    void TestHash();
    void TestKey();
    void TestList();
    void TestSet();
    void TestSort();
    void TestString();
    void TestPubSub();

  protected:
    RedisClient mxRedisClient;
};
