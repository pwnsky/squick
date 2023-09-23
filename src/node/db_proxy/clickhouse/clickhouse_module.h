#include "i_clickhouse_module.h"
#include <squick/plugin/net/export.h>
#include <squick/plugin/config/export.h>
class ClickhouseModule : public IClickhouseModule {
  public:
    ClickhouseModule(IPluginManager *p);
    virtual ~ClickhouseModule();

    virtual bool Start();
    virtual bool AfterStart();
    virtual bool Update();
    virtual bool Destory();

    void Test();
    virtual bool Connect(std::string url) override;
private:
    INetModule* m_net_;
    IClassModule* m_class_;
    IElementModule* m_element_;
    ILogModule* m_log_;
};
