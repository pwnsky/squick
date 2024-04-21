@echo off
rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-12-14
rem Github: https://github.com/pwnsky/squick
rem Description: Build third party library

set build_version="debug"
set project_path=%~dp0\..
set third_party_path=%project_path%/third_party
set build_path=%project_path%\third_party\build
set boost_root=%project_path%\third_party\build\boost_1_81_0
set mongo_c_driver_install=%project_path%\third_party\build\mongo-c-driver\install
set hiredis_install=%project_path%\third_party\build\hiredis\install
cd %third_party_path%

mkdir build\include
mkdir build\lib
mkdir build\bin
mkdir build\protobuf
mkdir build\libevent
mkdir build\zlib
mkdir build\navigation
mkdir build\mysql-connector-cpp
mkdir build\mongo-c-driver
mkdir build\mongo-cxx-driver
mkdir build\hiredis
mkdir build\redis-plus-plus
mkdir build\clickhouse-cpp

rem build clickhouse-cpp
cd build/clickhouse-cpp
cmake ..\..\clickhouse-cpp -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install
cmake --build . 
cmake --install . --prefix=install --config Debug
xcopy /s /e /y install ..
cd %third_party_path%

rem build hiredis
cd build/hiredis
cmake ..\..\hiredis -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install
cmake --build . 
cmake --install . --prefix=install --config Debug
xcopy /s /e /y install ..
cd %third_party_path%

rem build redis-plus-plus
cd build/redis-plus-plus
cmake ..\..\redis-plus-plus -DCMAKE_PREFIX_PATH=%hiredis_install% -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install
cmake --build . 
cmake --install . --prefix=install --config Debug
xcopy /s /e /y install ..
cd %third_party_path%

rem build mongo-c-driver
cd build\mongo-c-driver
cmake ..\..\mongo-c-driver -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug
cmake --build . 
cmake --install . --prefix=install --config Debug
xcopy /s /e /y install ..
cd %third_party_path%


rem build mongo-cxx-driver
cd build\mongo-cxx-driver
cmake ..\..\mongo-cxx-driver -DCMAKE_CXX_STANDARD=17 -DCMAKE_PREFIX_PATH=%mongo_c_driver_install% -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug
cmake --build . 
cmake --install . --prefix=install --config Debug
xcopy /s /e /y install ..


rem build libevent
cd build/libevent
mkdir install
cmake ..\..\libevent -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=install
cmake --build . 
cmake --install . --prefix=install --config Debug
xcopy /s /e /y install ..
cd %third_party_path%


rem build protobuf
cd build/protobuf
cmake ..\..\protobuf -Dprotobuf_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=truess -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug
cmake --build . 
cmake --install . --prefix=./install --config Debug
xcopy /s /e /y install ..
cd %third_party_path%


rem build lua, not surport mingw32 to link vs project
rem cd lua/src
rem mingw32-make.exe mingw
rem xcopy /s /e /y lua.dll ..\..\build\lib
rem xcopy /s /e /y lua.lib ..\..\build\lib
rem cd %third_party_path%

rem build mysqlx


rem  build zlib
cd build/zlib
cmake  ..\..\zlib -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug
cmake --build . 
cmake --install . --prefix=./install --config Debug
xcopy /s /e /y install ..
cd %third_party_path%


rem navigation
cd build/navigation
cmake  ..\..\recastnavigation -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Debug
cmake --build . 
cmake --install . --prefix=./install --config Debug
xcopy /s /e /y Debug ..\lib
cd %third_party_path%


if "%1"=="no_pause" (
    echo continue
) else (
    pause
)

