#pragma once
#include <squick/core/date_time.h>
#include <squick/core/i_plugin_manager.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/kernel/i_kernel_module.h>
#include <squick/plugin/log/i_log_module.h>
#include <squick/plugin/net/i_net_module.h>
#include <squick/plugin/redis/export.h>
#include <squick/struct/struct.h>

#include "i_account_redis_module.h"
#include "i_common_redis_module.h"
#include "i_player_redis_module.h"

class PlayerRedisModule : public IPlayerRedisModule {
  public:
    PlayerRedisModule(IPluginManager *p);

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();

    virtual bool AfterStart();

    virtual bool ExistRoleName(const std::string &strRoleName);
    virtual bool CreateRole(const std::string &account, const std::string &strRoleName, const Guid &id, const int nHomeSceneID);
    virtual bool GetRoleInfo(const std::string &account, std::string &strRoleName, Guid &id);

    virtual bool LoadPlayerData(const Guid &self, SquickStruct::PlayerData &playerData);
    virtual bool SavePlayerData(const Guid &self, const SquickStruct::PlayerData &playerData);

  protected:
    std::string GetOnlineGameServerKey();
    std::string GetOnlineProxyServerKey();

  private:
  private:
    IClassModule *m_class_;
    IAccountRedisModule *m_pAccountRedisModule;
    INoSqlModule *m_pNoSqlModule;
    ICommonRedisModule *m_pCommonRedisModule;
    IKernelModule *m_kernel_;
    ILogModule *m_log_;
};