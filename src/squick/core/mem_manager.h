
#ifndef SQUICK_NFMEMMANGER_H
#define SQUICK_NFMEMMANGER_H

#include "platform.h"

#ifdef SQUICK_USE_TCMALLOC
#include "third_party/gperftools-2.7/src/gperftools/malloc_extension.h"
#endif

#ifdef SQUICK_USE_TCMALLOC

#ifdef SQUICK_DEBUG_MODE

#if PLATFORM == PLATFORM_WIN
#pragma comment(lib, "libtcmalloc_minimal.lib")
#pragma comment(linker, "/include:__tcmalloc")
#elif PLATFORM == PLATFORM_LINUX || PLATFORM == PLATFORM_ANDROID
#pragma comment(lib, "libtcmalloc_minimal.a")
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
#endif

#else

#if PLATFORM == PLATFORM_WIN
#pragma comment(lib, "libtcmalloc_minimal.lib")
#pragma comment(linker, "/include:__tcmalloc")
#elif PLATFORM == PLATFORM_LINUX || PLATFORM == PLATFORM_ANDROID
#pragma comment(lib, "libtcmalloc_minimal.a")
#elif PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_APPLE_IOS
#endif

#endif // SQUICK_USE_TCMALLOC

#endif

class MemManager {
  public:
    MemManager() {}

    ~MemManager() {}

  public:
    void FreeMem() {

#ifdef SQUICK_USE_TCMALLOC

        MallocExtension::instance()->ReleaseFreeMemory();
#endif
    }

  private:
};

#endif