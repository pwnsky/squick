#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-09-25
# Github: https://github.com/i0gan/Squick
# Description: Build third party library

# 编译protobuf
#cd protobuf-21.6

third_party_path=`pwd`/../third_party
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


# build libevent
cd build/libevent
cmake  ../../libevent
make -j $(nproc)
mkdir -p ./install && make install DESTDIR=./install
cp -r install/usr/local/include/* ../include
#cp -r ./include/* ../include
cd ./lib
cp *.a ../../lib
cp *.so ../../lib
cd $third_party_path


# build protobuf
cd build/protobuf
cmake ../../protobuf -Dprotobuf_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=true
make -j $(nproc)
mkdir -p ./install && make install DESTDIR=./install
cp -r install/usr/local/include/* ../include
cp -r install/usr/local/bin/* ../bin
cp libprotobuf.so.3.21.6.0 ../lib/libprotobuf.so
cd $third_party_path


# build lua
cd lua
make linux
cp ./src/*.h ../build/include/
cp ./src/*.hpp ../build/include/
cp ./src/*.a ../build/lib
cp ./src/*.so ../build/lib
cd $third_party_path



# build redis connector
cd hiredis
make clean
make
cp *.a $third_party_path/build/lib
cp *.so $third_party_path/build/lib
cp *.h $third_party_path/build/include
cd $third_party_path


# build mysql connector
cd ./build/mysql-connector-cpp
cmake ../../mysql-connector-cpp
make -j $(nproc)
make -j $(nproc) # 再编译一次，protobuf内存不足的话会编译失败
mkdir -p ./install && make install DESTDIR=./install
cp -r install/usr/local/mysql/connector-c++-8.0.31/include/* ../include
cp libmysqlcppconn8.so ../lib/
cd $third_party_path


# build zlib
cd build/zlib
cmake  ../../zlib
make -j $(nproc)
cp *.a $third_party_path/build/lib
cp *.so $third_party_path/build/lib
cp *.h $third_party_path/build/include
cd $third_party_path


# build navigation
cd build/navigation
cmake  ../../recastnavigation
make -j $(nproc)
cp *.a $third_party_path/build/lib
cp *.so $third_party_path/build/lib
#cp *.h $third_party_path/build/include
cd $third_party_path
