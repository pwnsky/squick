#pragma once

#include <google/protobuf/dynamic_message.h>
#include <squick/core/base.h>

#include <squick/plugin/config/export.h>
#include <squick/plugin/kernel/export.h>
#include <squick/plugin/net/export.h>

#include "../server/i_server_module.h"

namespace robot::logic {
class IRobotManagerModule : public IModule {};
} // namespace robot::logic
