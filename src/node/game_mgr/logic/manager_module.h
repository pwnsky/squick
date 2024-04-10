#pragma once
#include <squick/core/platform.h>
#include <squick/plugin/config/export.h>
#include <squick/plugin/log/export.h>
#include <squick/plugin/lua/export.h>
#include <squick/plugin/net/export.h>
#include <struct/struct.h>

namespace gameplay_manager::logic {
class IManagerModule : public IModule {};

class ManagerModule : public IManagerModule {
  public:
    ManagerModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true;
    }

    virtual bool Start();
    virtual bool Destroy();

    virtual bool AfterStart();
    virtual bool Update();

  protected:
    // 创建实例
    void InstanceCreate(const string &instanceId, const string &key);
    // 销毁实例
    void InstanceDestroy(const string &instanceId);

  private:
    IClassModule *m_class_;
    IElementModule *m_element_;
    ILogModule *m_log_;
};

} // namespace gameplay_manager::logic
