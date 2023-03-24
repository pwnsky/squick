#pragma once
// Author: i0gan
// Email : l418894113@gmail.com
// Date  : 2022-12-03
// Github: https://github.com/i0gan/Squick
// Description: Export base headers in a single header file

#include "i_module.h"
#include "i_plugin.h"
#include "i_plugin_manager.h"
#include <iostream>

enum SQUICK_SERVER_TYPES {
    SQUICK_ST_NONE = 0,              // NONE
    SQUICK_ST_MASTER = 1,            //
    SQUICK_ST_LOGIN = 2,             //
    SQUICK_ST_PROXY = 3,             //
    SQUICK_ST_GAME = 4,              //
    SQUICK_ST_WORLD = 5,             //
    SQUICK_ST_DB_PROXY = 6,          //
    SQUICK_ST_GAMEPLAY_MANAGER = 7,  //
    SQUICK_ST_GAMEPLAY = 8,          //
    SQUICK_ST_MICRO = 9,             // 微服务
    SQUICK_ST_CDN = 10,              // CDN
    
    SQUICK_ST_MAX = 11, // 最大服务类型
};

enum class DbType {
    None = 0,
    Mysql = 1,
    MongoDB = 2,
    Redis = 3,
};

//
#ifdef SQUICK_DEV
#define dout std::cout << "SQUICK_DEV LOG:" << __FILE__ << ":" << __LINE__ << " "
#define eout std::cout << "ERROR: "
#else
// Release版本时可以将 dout 打印给注释掉
#define stuff
#define dout / stuff /
#endif

#define dlog(x) "SQUICK_DEV LOG:" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "\n" + std::string(x) + "\n"