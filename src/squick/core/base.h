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

enum ServerType {
    ST_NONE = 0,             // NONE
    ST_MASTER = 1,           //
    ST_LOGIN = 2,            //
    ST_PROXY = 3,            //
    ST_GAME = 4,             //
    ST_WORLD = 5,            //
    ST_DB_PROXY = 6,         //
    ST_GAMEPLAY_MANAGER = 7, //
    ST_GAMEPLAY = 8,         //
    ST_LOBBY = 9,
    ST_MICRO = 10, // 微服务
    ST_CDN = 11,   // CDN
    ST_ROBOT = 12, // 机器人: 模拟玩家
    ST_MAX = 13,   // 最大服务类型
};

enum class DbType {
    None = 0,
    Mysql = 1,
    MongoDB = 2,
    Redis = 3,
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
#undef SQUICK_DEV
#ifdef SQUICK_DEV
#define dout std::cout << "SQUICK_DEV LOG:" << __FILE__ << ":" << __LINE__ << " "
#define eout std::cout << "ERROR: "
#else
// Release版本时可以将 dout 打印给注释掉
#define stuff
#define dout /stuff/
#endif

#define dlog(x) "SQUICK_DEV LOG:" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "\n" + std::string(x) + "\n"