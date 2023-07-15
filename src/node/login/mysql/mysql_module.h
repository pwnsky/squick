#pragma once

#include "squick/core/map.h"
#include <struct/struct.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>

#include "i_mysql_module.h"

#include <mysqlx/xdevapi.h>
#include <string>
namespace login::mysql {
using namespace ::mysqlx;
class MysqlModule : public IMysqlModule {
  public:
    MysqlModule(IPluginManager *p) { pm_ = p; }

    virtual bool Start();
    virtual bool Destory();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool AfterStart();
    virtual void OnLoginProcess(const socket_t sock, const int msg_id, const char *msg, const uint32_t len);

    virtual bool RegisterAccount(const std::string &guid, const std::string &account, const std::string &password) override;
    virtual bool IsHave(const std::string &column_name, const std::string &value) override;
    virtual Guid GetGuid(AccountType type, const std::string &account) override;

  private:
    bool Connect(const std::string &user, const std::string &password, const std::string &host, int port);
    std::string host_;
    int port_;
    std::string user_;
    std::string password_;
    std::string database_;

    Session *session_ = nullptr;

  protected:
    INetModule *m_net_;
    ILogModule *m_log_;
    IClassModule *m_class_;
    IElementModule *m_element_;

  private:
};

} // namespace login::mysql