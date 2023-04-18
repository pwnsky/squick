#include "i_mongo_module.h"

class MongoModule : public IMongoModule {
  public:
    MongoModule(IPluginManager *p);
    virtual ~MongoModule();

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();
    virtual bool Destory();

    void Test();
    virtual bool Connect(std::string url) override;
};
