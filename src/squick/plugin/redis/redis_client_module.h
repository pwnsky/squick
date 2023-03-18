#pragma once

#include "i_redis_client_module.h"
#include "redis_client.h"
#include <squick/core/i_plugin_manager.h>
#include <squick/core/platform.h>
#include <squick/plugin/config/i_class_module.h>
#include <squick/plugin/config/i_element_module.h>
#include <squick/plugin/log/i_log_module.h>

class NoSqlModule : public INoSqlModule {
  public:
    NoSqlModule(IPluginManager *p);
    virtual ~NoSqlModule();

    virtual bool Start();
    virtual bool Destory();
    virtual bool Update();
    virtual bool AfterStart();

    virtual bool Connect(const std::string &ip, const int nPort, const std::string &strPass) { return false; };
    virtual bool Enable();
    virtual bool Busy();
    virtual bool KeepLive();

    virtual bool AddConnectSql(const std::string &strID, const std::string &ip);
    virtual bool AddConnectSql(const std::string &strID, const std::string &ip, const int nPort);
    virtual bool AddConnectSql(const std::string &strID, const std::string &ip, const int nPort, const std::string &strPass);

    virtual List<std::string> GetDriverIdList();
    virtual SQUICK_SHARE_PTR<IRedisClient> GetDriver(const std::string &strID);
    virtual SQUICK_SHARE_PTR<IRedisClient> GetDriverBySuitRandom();
    virtual SQUICK_SHARE_PTR<IRedisClient> GetDriverBySuitConsistent();
    virtual SQUICK_SHARE_PTR<IRedisClient> GetDriverBySuit(const std::string &strHash);
    // virtual SQUICK_SHARE_PTR<IRedisClient> GetDriverBySuit(const int nHash);
    virtual bool RemoveConnectSql(const std::string &strID);

  protected:
    void CheckConnect();

  protected:
    INT64 mLastCheckTime;
    IClassModule *m_pClassModule;
    IElementModule *m_pElementModule;
    ILogModule *m_pLogModule;

    NFConsistentHashMapEx<std::string, IRedisClient> mxNoSqlDriver;
};
