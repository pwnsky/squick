#include "i_mongo_module.h"
#include <squick/plugin/net/export.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
class MongoModule : public IMongoModule {
  public:
    MongoModule(IPluginManager *p);
    virtual ~MongoModule();

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();
    virtual bool Destory();
    virtual bool Connect();
private:
    INetModule* m_net_;
    IClassModule* m_class_;
    IElementModule* m_element_;
    ILogModule* m_log_;
};
