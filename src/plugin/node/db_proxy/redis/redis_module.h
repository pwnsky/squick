#include "i_redis_module.h"
#include <plugin/core/net/export.h>
#include <sw/redis++/redis++.h>

namespace db_proxy::redis {
using namespace sw::redis;
class RedisModule : public IRedisModule {
  public:
    RedisModule(IPluginManager *p);
    virtual ~RedisModule();
    virtual bool AfterStart();
    virtual bool Update();
    virtual bool Destroy();
    virtual bool Connect();

    void OnReqQuery(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    void OnReqRedisGet(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    void OnReqRedisSet(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    void OnReqRedisHGet(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    void OnReqRedisHSet(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    void OnReqRedisHGetAll(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);
    void OnReqRedisHMSet(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len);

  private:
    Redis *client_;
};

} // namespace db_proxy::redis