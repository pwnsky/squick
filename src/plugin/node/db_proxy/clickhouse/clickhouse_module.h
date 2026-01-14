#include "i_clickhouse_module.h"
#include <plugin/core/config/export.h>
#include <plugin/core/net/export.h>

#include <clickhouse/client.h>
using namespace clickhouse;
class ClickhouseModule : public IClickhouseModule {
  public:
    ClickhouseModule(IPluginManager *p);
    virtual ~ClickhouseModule();

    void OnReqQuery(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    void OnReqExecute(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    void OnReqInsert(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    void OnReqSelect(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);

    virtual bool AfterStart();
    virtual bool Update();
    virtual bool Destroy();

    void Test();
    virtual bool Connect();

  private:
    Client *client_ = nullptr;
};
