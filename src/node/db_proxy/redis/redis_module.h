#include "i_redis_module.h"
#include "../node/i_node_module.h";
#include <squick/plugin/net/export.h>
#include <sw/redis++/redis++.h>


namespace db_proxy::redis {
using namespace sw::redis;
class RedisModule : public IRedisModule {
  public:
    RedisModule(IPluginManager *p);
    virtual ~RedisModule();

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();
    virtual bool Destory();

    void Test();
    virtual bool Connect();

    void OnReqQuery(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqRedisGet(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqRedisSet(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqRedisHGet(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqRedisHSet(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqRedisHGetAll(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);
    void OnReqRedisHMSet(const socket_t sock, const int msg_id, const char* msg, const uint32_t len);

    Redis* client_;
private:
    INetModule* m_net_;
    IClassModule *m_class_;
    IElementModule *m_element_;
    ILogModule *m_log_;
    node::INodeModule* m_node_;

    
};

}