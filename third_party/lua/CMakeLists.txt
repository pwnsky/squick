# CMake 最低版本要求
cmake_minimum_required(VERSION 3.15)

# 项目名称与版本
project(Lua5.5 LANGUAGES C VERSION 5.5.1)

# 配置编译选项
set(CMAKE_C_STANDARD 99)  # Lua 源码遵循 C99 标准
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # 生成位置无关代码，支持动态库链接

# 屏蔽不必要的警告（跨平台兼容）
if(MSVC)
    # Windows MSVC 编译器：屏蔽冗余警告
    add_compile_options(/W3 /wd4996 /wd4244)
else()
    # Linux/macOS GCC/Clang 编译器：优化级别 + 屏蔽冗余警告
    add_compile_options(-O2 -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare)
endif()

# 定义 Lua 编译宏（遵循官方源码的编译定义）
add_definitions(
    -DLUA_COMPAT_5_3  # 兼容 Lua 5.3 语法和 API
    #-DLUA_USE_POSIX   # 类 Unix 系统启用 POSIX 特性（Windows 会自动忽略）
)

# 筛选 Lua 核心源码文件（排除 luac.c 编译解释器，排除 lua.c 编译库文件）
file(GLOB LUA_CORE_SRCS "${PROJECT_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM LUA_CORE_SRCS
    "${PROJECT_SOURCE_DIR}/src/luac.c"  # 编译 Lua 编译器（可选，此处仅编译核心库）
    "${PROJECT_SOURCE_DIR}/src/lua.c"   # 编译 Lua 交互式解释器（单独构建可执行文件）
)

# ---------------------- 1. 构建 Lua 静态库（默认优先） ----------------------
add_library(lua_static STATIC ${LUA_CORE_SRCS})
# 设置静态库输出名称（统一为 lua，方便链接）
set_target_properties(lua_static PROPERTIES
    OUTPUT_NAME "lua"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"  # 静态库输出目录
)
# 配置静态库的头文件目录（让使用者可以 #include <lua.h>）
target_include_directories(lua_static
    PUBLIC "${PROJECT_SOURCE_DIR}/src"
)

# ---------------------- 2. 构建 Lua 动态库（可选，按需启用） ----------------------
option(BUILD_LUA_SHARED "Build Lua shared library (dynamic library)" OFF)
if(BUILD_LUA_SHARED)
    add_library(lua_shared SHARED ${LUA_CORE_SRCS})
    set_target_properties(lua_shared PROPERTIES
        OUTPUT_NAME "lua"
        LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"  # 动态库输出目录（Linux/macOS）
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"  # 动态库输出目录（Windows .dll）
        VERSION ${PROJECT_VERSION}
        SOVERSION 5.5  # 动态库版本号（遵循 Linux 库版本规范）
    )
    target_include_directories(lua_shared
        PUBLIC "${PROJECT_SOURCE_DIR}/src"
    )
    # Windows 动态库需要导出符号
    if(WIN32)
        target_compile_definitions(lua_shared PRIVATE -DLUA_BUILD_AS_DLL)
    endif()
endif()

# ---------------------- 3. 构建 Lua 交互式解释器（可选，方便测试） ----------------------
option(BUILD_LUA_INTERPRETER "Build Lua interactive interpreter (lua.exe/lua)" ON)
if(BUILD_LUA_INTERPRETER)
    add_executable(lua_interpreter "${PROJECT_SOURCE_DIR}/src/lua.c")
    set_target_properties(lua_interpreter PROPERTIES
        OUTPUT_NAME "lua"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"  # 可执行文件输出目录
    )
    # 链接 Lua 静态库（优先）
    target_link_libraries(lua_interpreter PRIVATE lua_static)
    # 跨平台链接系统库（类 Unix 系统需要链接 readline 和 m 数学库）
    if(UNIX AND NOT APPLE)
        target_link_libraries(lua_interpreter PRIVATE readline m)
    elseif(APPLE)
        target_link_libraries(lua_interpreter PRIVATE readline m)
    endif()
endif()

# ---------------------- 4. 安装配置（可选，支持 make install 部署） ----------------------
option(INSTALL_LUA "Install Lua library and headers" OFF)
if(INSTALL_LUA)
    # 安装头文件
    install(FILES
        "${PROJECT_SOURCE_DIR}/src/lua.h"
        "${PROJECT_SOURCE_DIR}/src/lualib.h"
        "${PROJECT_SOURCE_DIR}/src/luaconf.h"
        "${PROJECT_SOURCE_DIR}/src/lauxlib.h"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/include/lua5.5"
    )
    # 安装静态库
    install(TARGETS lua_static
        ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
    )
    # 安装动态库（若启用）
    if(BUILD_LUA_SHARED)
        install(TARGETS lua_shared
            LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
            RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
        )
    endif()
    # 安装解释器（若启用）
    if(BUILD_LUA_INTERPRETER)
        install(TARGETS lua_interpreter
            RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
        )
    endif()
endif()