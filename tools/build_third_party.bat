rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-12-14
rem Github: https://github.com/i0gan/Squick
rem Description: Build third party library

set build_version="release"
set project_path=%~dp0\..
set third_party_path=%project_path%/third_party
set build_path=%project_path%\third_party\build
cd %third_party_path%


mkdir build\include
mkdir build\lib
mkdir build\bin
mkdir build\protobuf
mkdir build\libevent
mkdir build\zlib
mkdir build\navigation
mkdir build\mysql-connector-cpp


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


rem build lua
cd lua/src
mingw32-make.exe mingw
xcopy /s /e /y lua.dll ..\..\build\lib
xcopy /s /e /y lua.lib ..\..\build\lib
cd %third_party_path%


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

pause
