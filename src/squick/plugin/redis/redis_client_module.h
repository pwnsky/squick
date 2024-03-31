#pragma once

#include "i_redis_client_module.h"
#include "redis_client.h"
#include <squick/core/i_plugin_manager.h>
#include <squick/core/platform.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/log/i_log_module.h>

class RedisModule : public IRedisModule {
  public:
    RedisModule(IPluginManager *p);
    virtual ~RedisModule();

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

    virtual bool Connect(const std::string &ip, const int nPort, const std::string &strPass) { return false; };
    virtual bool Enable();
    virtual bool Busy();
    virtual bool KeepLive();
    virtual bool AddConnectSql(const std::string &strID, const std::string &ip, const int nPort, const std::string &strPass);

    virtual List<std::string> GetDriverIdList();
    virtual std::shared_ptr<IRedisClient> GetDriver(const std::string &strID);
    virtual std::shared_ptr<IRedisClient> GetDriverBySuitRandom();
    virtual std::shared_ptr<IRedisClient> GetDriverBySuitConsistent();
    virtual std::shared_ptr<IRedisClient> GetDriverBySuit(const std::string &strHash);
    // virtual std::shared_ptr<IRedisClient> GetDriverBySuit(const int nHash);
    virtual bool RemoveConnectSql(const std::string &strID);

  protected:
    void CheckConnect();

  protected:
    INT64 mLastCheckTime;
    ILogModule *m_log_;

    ConsistentHashMapEx<std::string, IRedisClient> mdriver;
};
