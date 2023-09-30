#include "i_clickhouse_module.h"
#include <squick/plugin/net/export.h>
#include <squick/plugin/config/export.h>

#include <clickhouse/client.h>
using namespace clickhouse;
class ClickhouseModule : public IClickhouseModule {
  public:
    ClickhouseModule(IPluginManager *p);
    virtual ~ClickhouseModule();

    void OnReqQuery(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqExecute(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqInsert(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqSelect(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);


    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();
    virtual bool Destory();

    void Test();
    virtual bool Connect();
private:
    Client* client_;
    INetModule* m_net_;
    IClassModule* m_class_;
    IElementModule* m_element_;
    ILogModule* m_log_;
};
