#ifndef SQUICK_DYNLIB_H
#define SQUICK_DYNLIB_H

#include <stdio.h>
#include <iostream>
#include "platform.h"

//struct HINSTANCE__;
//typedef struct HINSTANCE__* hInstance;
#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
#    define DYNLIB_HANDLE hInstance
#    define DYNLIB_LOAD( a ) LoadLibraryExA( a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH )
#    define DYNLIB_GETSYM( a, b ) GetProcAddress( a, b )
#    define DYNLIB_UNLOAD( a ) FreeLibrary( a )

struct HINSTANCE__;
typedef struct HINSTANCE__* hInstance;

#elif SQUICK_PLATFORM == SQUICK_PLATFORM_LINUX || SQUICK_PLATFORM == SQUICK_PLATFORM_ANDROID
#include <dlfcn.h>
#define DYNLIB_HANDLE void*
#define DYNLIB_LOAD( a ) dlopen( a, RTLD_LAZY | RTLD_GLOBAL)
#define DYNLIB_GETSYM( a, b ) dlsym( a, b )
#define DYNLIB_UNLOAD( a ) dlclose( a )

#elif SQUICK_PLATFORM == SQUICK_PLATFORM_APPLE || SQUICK_PLATFORM == SQUICK_PLATFORM_APPLE_IOS
#include <dlfcn.h>
#define DYNLIB_HANDLE void*
#define DYNLIB_LOAD( a ) dlopen( a, RTLD_LOCAL|RTLD_LAZY)
#define DYNLIB_GETSYM( a, b ) dlsym( a, b )
#define DYNLIB_UNLOAD( a ) dlclose( a )

#endif

class DynLib
{

public:

    DynLib(const std::string& name) {
        mbMain = false;
        mstrName = "plugin/";
        mstrName.append(name);
#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
        mstrName.append(".dll");
#else
        mstrName.append(".so");
#endif
#ifdef DEBUG
        printf("Load Plugin from :%s\n", mstrName.c_str());
#endif
    }

    ~DynLib()
    {
    }

    bool Load();

    bool UnLoad();

    /// Get the name of the library
    const std::string& GetName(void) const
    {
        return mstrName;
    }

    const bool GetMain(void) const
    {
        return mbMain;
    }

    void* GetSymbol(const char* szProcName);

protected:

    std::string mstrName;
    bool mbMain;

    DYNLIB_HANDLE mInst;
};

#endif
