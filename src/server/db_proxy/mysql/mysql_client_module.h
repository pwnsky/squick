#include "i_mysql_client_module.h"

class MysqlModule : public IMysqlModule {
  public:
    MysqlModule(IPluginManager *p);
    virtual ~MysqlModule();

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();
    virtual bool Destory();

    void Test();
    virtual bool Connect(std::string url) override;
};
