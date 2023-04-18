

#include "robot_manager_module.h"
#include <server/db_proxy/logic/common_redis_module.h>

namespace robot::logic {

bool RobotManagerModule::Start() { return true; }

bool RobotManagerModule::AfterStart() { return true; }

bool RobotManagerModule::ReadyUpdate() { return true; }

bool RobotManagerModule::Destory() { return true; }

bool RobotManagerModule::Update() { return true; }

} // namespace robot::logic