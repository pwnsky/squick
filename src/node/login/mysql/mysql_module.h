#pragma once

#include "squick/core/map.h"
#include <struct/struct.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <squick/imodule/i_mysql_module.h>
#include "i_mysql_module.h"

#include <mysqlx/xdevapi.h>
#include <string>
namespace login::mysql {
using namespace ::mysqlx;
class IMysqlModuleA : public IMysqlModule, public imodule::IMysqlModule {
    virtual bool Start() {
        return imodule::IMysqlModule::Start();;
    }
    virtual bool Destory() {
        return imodule::IMysqlModule::Destory();
    }
    virtual bool ReadyUpdate() {
        return imodule::IMysqlModule::ReadyUpdate();
    }
    virtual bool Update() {
        return imodule::IMysqlModule::Update();
    }
    virtual bool AfterStart() {
        return imodule::IMysqlModule::AfterStart();
    }
};

class MysqlModule : public IMysqlModuleA {
  public:
    MysqlModule(IPluginManager* p) { pm_ = p; is_update_ = true; }
    virtual void OnLoginProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);
    virtual bool RegisterAccount(const std::string &guid, const std::string &account, const std::string &password) override;
    virtual bool IsHave(const std::string &column_name, const std::string &value) override;
    virtual Guid GetGuid(AccountType type, const std::string &account) override;
  private:
};

} // namespace login::mysql