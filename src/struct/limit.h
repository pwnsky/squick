#pragma once

#include <squick/core/platform.h>

#define SQUICK_NAME_MAXLEN 32
#define SQUICK_BROAD_MAXLEN 64
#define SQUICK_PATH_MAXLEN 128
#define SQUICK_ACCOUNT_MAXROLE 4
#define SQUICK_ROLE_MAXPKG 150
#define SQUICK_ROLE_DEFAULTPKG 60

#define SQUICK_VIEWGRID_UNIT 200
#define SQUICK_VIEWSIZE_MAXLEN 9

// #define SQUICK_SCENE_LEFTLENPOS       -32000
// #define SQUICK_SCENE_RIGHTPOS     32000

// #define SQUICK_MAX_ONLINE         3000

#ifdef SQUICK_DEBUG_MODE

#if PLATFORM == PLATFORM_WIN

#elif PLATFORM == PLATFORM_LINUX || PLATFORM == PLATFORM_ANDROID

#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
#endif

#else

#if PLATFORM == PLATFORM_WIN

#elif PLATFORM == PLATFORM_LINUX || PLATFORM == PLATFORM_ANDROID

#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
#endif

#endif

class _SquickExport DoNothing {
  public:
    void DonOTHING() {}
};