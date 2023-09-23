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

    Redis* redis_;
private:
    INetModule* m_net_;
    IClassModule *m_class_;
    IElementModule *m_element_;
    ILogModule *m_log_;
    node::INodeModule* m_node_;

    
};

}