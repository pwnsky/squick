#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-09-25
# Github: https://github.com/pwnsky/squick
# Description: Build third party library, run on linux and macOS

# for docker env
cd $(dirname $0)
source common.sh
git config --global --add safe.directory /mnt

third_party_path=`pwd`/../third_party
mongo_c_driver_install=$third_party_path/build/mongo-c-driver/install
hiredis_install=$third_party_path/build/hiredis/install

function reset_env()
{
    log_debug "Reset build env ..."
    
    cd $third_party_path
    rm -rf ./build

    mkdir -p build/include
    mkdir -p build/lib
    mkdir -p build/bin
    mkdir -p build/protobuf
    mkdir -p build/libevent
    mkdir -p build/zlib
    mkdir -p build/navigation
    mkdir -p build/mysql-connector-cpp
    mkdir -p build/mongo-c-driver
    mkdir -p build/mongo-cxx-driver
    mkdir -p build/hiredis
    mkdir -p build/redis-plus-plus
    mkdir -p build/clickhouse-cpp
    check_err
}



function build_clickhouse()
{
    log_debug "Rbuild clickhouse"
    cd $third_party_path
    cd build/clickhouse-cpp
    cmake ../../clickhouse-cpp -DBUILD_SHARED_LIBS=true -DCMAKE_INSTALL_PREFIX=install
    cmake --build . -j $make_threads
    check_err
    cmake --install . 
    cp install/lib/* ../lib
    cp -r install/include/* ../include
}


function build_hredis()
{
    log_debug "build_hredis"
    cd $third_party_path
    cd build/hiredis
    cmake ../../hiredis -DBUILD_SHARED_LIBS=true -DCMAKE_INSTALL_PREFIX=install
    cmake --build . -j $make_threads
    check_err
    cmake --install . 
    cp install/lib/* ../lib
    cp -r install/include/* ../include
}


function build_redis_plus_plus()
{
    log_debug "build redis-plus-plus"
    cd $third_party_path
    cd build/redis-plus-plus
    cmake ../../redis-plus-plus -DBUILD_SHARED_LIBS=true -DCMAKE_INSTALL_PREFIX=install -DCMAKE_PREFIX_PATH=$hiredis_install
    cmake --build . -j $make_threads
    check_err
    cmake --install . 
    cp install/lib/* ../lib
    cp -r install/include/* ../include
}

function build_mongo_c_driver()
{
    log_debug "build mongo c driver"
    cd $third_party_path
    cd build/mongo-c-driver
    cmake ../../mongo-c-driver -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=$build_type
    cmake --build . -j $make_threads
    check_err
    cmake --install . 
    cp install/lib/* ../lib
}


function build_mongo_cxx_driver()
{
    log_debug "build mongo cxx driver"
    cd $third_party_path
    cd build/mongo-cxx-driver
    cmake ../../mongo-cxx-driver -DCMAKE_CXX_STANDARD=17 -DCMAKE_PREFIX_PATH=$mongo_c_driver_install -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=$build_type
    cmake --build . -j $make_threads
    check_err
    cmake --install .
    cp -r install/include/bsoncxx/v_noabi/* ../include
    cp -r install/include/mongocxx/v_noabi/* ../include
    cp install/lib/* ../lib
}


function build_libevent()
{
    log_debug "build libevent"
    cd $third_party_path
    cd build/libevent
    cmake  ../../libevent
    cmake --build . -j $make_threads
    check_err
    mkdir -p ./install && make install DESTDIR=./install
    cp -r install/usr/local/include/* ../include
    #cp -r ./include/* ../include
    cp ./lib/* ../lib
}


function build_protobuf()
{
    log_debug "build protobuf"
    cd $third_party_path
    cd build/protobuf
    cmake ../../protobuf -Dprotobuf_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=true
    cmake --build . -j $make_threads
    check_err
    mkdir -p ./install && make install DESTDIR=./install
    cp -r install/usr/local/include/* ../include
    cp -r install/usr/local/bin/* ../bin
    cp -r install/usr/local/lib/* ../lib
}


function build_lua()
{
    log_debug "build lua"
    cd $third_party_path
    cd lua
    if [ $sys == "Darwin" ];then
        make macosx
        check_err
        cp ./src/*.dylib ../build/lib
    else
        make linux CFLAGS="-fPIC"
        check_err
        cp ./src/*.so ../build/lib
    fi
    cp ./src/*.h ../build/include/
    cp ./src/*.hpp ../build/include/
    cp ./src/*.a ../build/lib
}

function build_mysql_connector()
{
    log_debug "build mysql connector"
    cd $third_party_path
    cd ./build/mysql-connector-cpp
    cmake ../../mysql-connector-cpp
    cmake --build . -j $make_threads
    check_err
    mkdir -p ./install && make install DESTDIR=./install
    cp -r install/usr/local/mysql/connector-c++-8.0.31/include/* ../include
    if [ $sys == "Darwin" ];then
        cp libmysqlcppconn8.dylib ../lib/
    else
        cp libmysqlcppconn8.so ../lib/
    fi
}

function build_zlib()
{
    log_debug "build zlib"
    cd $third_party_path
    cd build/zlib
    cmake  ../../zlib
    cmake --build . -j $make_threads
    check_err
    cp *.a $third_party_path/build/lib
    cp *.h $third_party_path/build/include
    if [ $sys == "Darwin" ];then
        cp *.dylib $third_party_path/build/lib
    else
        cp *.so $third_party_path/build/lib
    fi
}


function build_navigation()
{
    log_debug "build navigation"
    cd $third_party_path
    cd build/navigation
    cmake  ../../recastnavigation
    cmake --build . -j $make_threads
    check_err
    cp *.a $third_party_path/build/lib
}

# fix change mode
function chmod_lib()
{
    log_debug "chmod lib"
    cd $third_party_path
    if [ $sys == "Darwin" ];then
        chmod +x $third_party_path/build/lib/*.dylib
    else
        chmod +x $third_party_path/build/lib/*.so
    fi
}

function clean_cache()
{
    log_debug "clean cache files"
    cd $third_party_path/build
    rm -rf clickhouse-cpp hiredis libevent mongo-cxx-driver mysql-connector-cpp navigation protobuf redis-plus-plus zlib mongo-c-driver
}

function main()
{
    time reset_env
    time build_clickhouse
    time build_hredis
    time build_redis_plus_plus
    time build_mongo_c_driver
    time build_mongo_cxx_driver
    time build_libevent
    time build_protobuf
    time build_lua
    time build_mysql_connector
    time build_zlib
    time build_navigation
    time chmod_lib
    time clean_cache
}

time main
log_debug "All library builded!"
