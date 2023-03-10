#pragma once

#include <squick/core/platform.h>
#include <squick/core/i_record_manager.h>
#include <squick/core/i_property_manager.h>
#include <squick/core/list.h>
#include "limit.h"

#include "base.pb.h"
#include "game_base.pb.h"
#include "game_play.pb.h"
#include "game_lobby.pb.h"
#include "game_manager.pb.h"
#include "server.pb.h"
#include "proxy.pb.h"
#include "db.pb.h"
#include "excel.h"
#include "game_play.pb.h"

// -------------------------------------------------------------------------
#pragma pack(push,1)

enum E_CHECK_TYPE
{
    ECT_SAVE        = 0,
    ECT_PRIVATE     = 1,
    ECT_PUBLIC      = 2,
};
#pragma pack(pop)
