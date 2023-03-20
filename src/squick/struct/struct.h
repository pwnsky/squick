#pragma once

#include "limit.h"
#include <squick/core/i_property_manager.h>
#include <squick/core/i_record_manager.h>
#include <squick/core/list.h>
#include <squick/core/platform.h>

#include "base.pb.h"
#include "db_proxy.pb.h"
#include "excel.h"
#include "game_base.pb.h"
#include "game_lobby.pb.h"
#include "game_manager.pb.h"
#include "game_play.pb.h"
#include "proxy.pb.h"
#include "server.pb.h"

// -------------------------------------------------------------------------
#pragma pack(push, 1)

enum E_CHECK_TYPE {
    ECT_SAVE = 0,
    ECT_PRIVATE = 1,
    ECT_PUBLIC = 2,
};
#pragma pack(pop)
