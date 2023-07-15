#pragma once

#include <squick/core/base.h>

#include "i_robot_manager_module.h"
#include <queue>
#include <struct/struct.h>
namespace robot::logic {

// 采用工厂模式来进行管理gameplay
class RobotManagerModule : public IRobotManagerModule {
  public:
    RobotManagerModule(IPluginManager *p) {
        pm_ = p;
        is_update_ = true; // Update
    }

    virtual ~RobotManagerModule(){};
    virtual bool Start();
    virtual bool Destory();
    virtual bool ReadyUpdate();
    virtual bool Update();
    virtual bool AfterStart();

  private:
};

} // namespace robot::logic