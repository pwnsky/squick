#pragma once

#include <google/protobuf/dynamic_message.h>
#include <squick/core/base.h>

#include <squick/plugin/config/export.h>
#include <squick/plugin/net/export.h>

#include "../node/i_node_module.h"

namespace robot::logic {
class IRobotManagerModule : public IModule {};
} // namespace robot::logic
