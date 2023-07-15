#pragma once

#include <squick/core/base.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/redis/export.h>
#include <struct/struct.h>

#include "i_account_redis_module.h"
#include "i_common_redis_module.h"
#include "i_player_redis_module.h"

class AccountRedisModule : public IAccountRedisModule {
  public:
    AccountRedisModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

    virtual bool VerifyAccount(const std::string &account, const std::string &strPwd);
    virtual bool AddAccount(const std::string &account, const std::string &strPwd);
    virtual bool ExistAccount(const std::string &account);

  protected:
  private:
    IClassModule *m_class_;
    IRedisModule *m_redis_;
    ICommonRedisModule *m_pCommonRedisModule;
    IKernelModule *m_kernel_;
};
