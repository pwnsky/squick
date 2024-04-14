#pragma once

#include "squick/core/map.h"
#include <struct/struct.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/net/export.h>
#include "i_mysql_module.h"

#include <mysqlx/xdevapi.h>
#include <string>
namespace db_proxy::mysql {
using namespace std;
class MysqlModule : public IMysqlModule {
public:
    MysqlModule(IPluginManager* p) { pm_ = p; is_update_ = true; }
    virtual bool AfterStart();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool Destroy();
private:
    bool Connect();
};

} // namespace login::mysql