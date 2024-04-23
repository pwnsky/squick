#pragma once

#include "i_mysql_module.h"
#include "squick/core/map.h"
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>

#include <mysqlx/xdevapi.h>
#include <string>
namespace db_proxy::mysql {
using namespace std;
using namespace ::mysqlx;
class MysqlModule : public IMysqlModule {
  public:
    MysqlModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }
    virtual bool AfterStart();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool Destroy();

    void OnReqExecute(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqSelect(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqInsert(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqUpdate(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);

  private:
    bool Connect();
    Session* session_ = nullptr;
};

} // namespace db_proxy::mysql