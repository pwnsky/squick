#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-09-25
# Github: https://github.com/i0gan/Squick
# Description: Build third party library, run on linux and macOS

# 编译protobuf
#cd protobuf-21.6

# for docker env
git config --global --add safe.directory /mnt

third_party_path=`pwd`/../third_party
mongo_c_driver_install=$third_party_path/build/mongo-c-driver/install
hiredis_install=$third_party_path/build/hiredis/install
build_type=Release
sys=`uname -s`

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

# build clickhouse
cd $third_party_path
cd build/clickhouse-cpp
cmake ../../clickhouse-cpp -DBUILD_SHARED_LIBS=true -DCMAKE_INSTALL_PREFIX=install
cmake --build . -j $(nproc)
cmake --install . 
cp install/lib/* ../lib
cp -r install/include/* ../include

# build hredis
cd $third_party_path
cd build/hiredis
cmake ../../hiredis -DBUILD_SHARED_LIBS=true -DCMAKE_INSTALL_PREFIX=install
cmake --build . -j $(nproc)
cmake --install . 
cp install/lib/* ../lib
cp -r install/include/* ../include

# build redis-plus-plus
cd $third_party_path
cd build/redis-plus-plus
cmake ../../redis-plus-plus -DBUILD_SHARED_LIBS=true -DCMAKE_INSTALL_PREFIX=install -DCMAKE_PREFIX_PATH=$hiredis_install
cmake --build . -j $(nproc)
cmake --install . 
cp install/lib/* ../lib
cp -r install/include/* ../include

# build mongo-c-driver
cd $third_party_path
cd build/mongo-c-driver
cmake ../../mongo-c-driver -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=$build_type
cmake --build . -j $(nproc)
cmake --install . 
#cp install/* 
cp install/lib/* ../lib


# build mongo-cxx-driver
cd $third_party_path
cd build/mongo-cxx-driver
cmake ../../mongo-cxx-driver -DCMAKE_CXX_STANDARD=17 -DCMAKE_PREFIX_PATH=$mongo_c_driver_install -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=$build_type
cmake --build . -j $(nproc)
cmake --install .
cp -r install/include/bsoncxx/v_noabi/* ../include
cp -r install/include/mongocxx/v_noabi/* ../include
cp install/lib/* ../lib


# build libevent
cd $third_party_path
cd build/libevent
cmake  ../../libevent
cmake --build . -j $(nproc)
mkdir -p ./install && make install DESTDIR=./install
cp -r install/usr/local/include/* ../include
#cp -r ./include/* ../include
cp ./lib/* ../lib


# build protobuf
cd $third_party_path
cd build/protobuf
cmake ../../protobuf -Dprotobuf_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=true
cmake --build . -j $(nproc)
mkdir -p ./install && make install DESTDIR=./install
cp -r install/usr/local/include/* ../include
cp -r install/usr/local/bin/* ../bin
cp -r install/usr/local/lib/* ../lib

# build lua
cd $third_party_path
cd lua
if [ $sys == "Darwin" ];then
    make macosx
    cp ./src/*.dylib ../build/lib
else
    make linux
    cp ./src/*.so ../build/lib
fi
cp ./src/*.h ../build/include/
cp ./src/*.hpp ../build/include/
cp ./src/*.a ../build/lib

# build mysql connector
cd $third_party_path
cd ./build/mysql-connector-cpp
cmake ../../mysql-connector-cpp
cmake --build . -j $(nproc)
mkdir -p ./install && make install DESTDIR=./install
cp -r install/usr/local/mysql/connector-c++-8.0.31/include/* ../include
if [ $sys == "Darwin" ];then
    cp libmysqlcppconn8.dylib ../lib/
else
    cp libmysqlcppconn8.so ../lib/
fi


# build zlib
cd $third_party_path
cd build/zlib
cmake  ../../zlib
cmake --build . -j $(nproc)
cp *.a $third_party_path/build/lib
cp *.h $third_party_path/build/include
if [ $sys == "Darwin" ];then
    cp *.dylib $third_party_path/build/lib
else
    cp *.so $third_party_path/build/lib
fi


# build navigation
cd $third_party_path
cd build/navigation
cmake  ../../recastnavigation
cmake --build . -j $(nproc)
cp *.a $third_party_path/build/lib

cd $third_party_path
# fix change mode
if [ $sys == "Darwin" ];then
    chmod +x $third_party_path/build/lib/*.dylib
else
    chmod +x $third_party_path/build/lib/*.so
fi
