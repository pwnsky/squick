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
#include "termcolor.h"

#define SQUICK_VERSION "1.1.0"
#define SERVER_NAME "Squick/" SQUICK_VERSION

enum ServerType {
    ST_NONE = 0,             // NONE
    ST_MASTER = 1,           //
    ST_LOGIN = 2,            //
    ST_WORLD = 3,            //
    ST_DB_PROXY = 4,         //
    ST_PROXY = 5,            //
    ST_PLAYER = 6,
    ST_GAME_MGR = 7, //
    ST_GAME = 8,     //
    ST_MICRO = 9,   // Micro
    ST_CDN = 10,    // CDN
    ST_ROBOT = 11,  // Robot, AI
    ST_GLOBAL = 12, // Global
    ST_ROOM = 12,   // Room
    ST_MATCH = 13,  // Match
    ST_GM = 14,     // GM
    ST_MAX = 15,
};

// 线程池 每一帧睡眠时间，单位毫秒
#define THREAD_POOL_SLEEP_TIME 100
// 主线程 每一帧睡眠时间，单位毫秒
#define MAIN_THREAD_SLEEP_TIME 1


#ifdef SQUICK_SSL
// 服务器证书 和 服务器私钥.
#define SQUICK_SSL_CERTIFICATE "../config/ssl/certificate.pem"
#define SQUICK_SSL_PRIVATE_KEY "../config/ssl/private-key.pem"
#endif

//
//#undef SQUICK_DEV
#ifdef SQUICK_DEV
#define dout std::cout << "SQUICK_DEV LOG:" << __FILE__ << ":" << __LINE__ << " "
#define eout std::cout << "ERROR: "
#else
// Release版本时可以将 dout 打印给注释掉
#define stuff
#define dout /stuff/
#endif

#define SQUICK_DPRINT(x) std::cout << termcolor::magenta << "SQUICK:"  << __FILE__ << ":" << __LINE__ << ":" << x << "\n" << termcolor::reset;
#define SQUICK_PRINT(x) std::cout << termcolor::magenta << "SQUICK:" << x << termcolor::reset << std::endl;

//#define SQUICK_PRINT SQUICK_DPRINT

SQUICK_EXPORT void SetSquickMainThreadSleep(bool is_sleep);
SQUICK_EXPORT bool IsSquickMainThreadSleep();