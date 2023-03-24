#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-09-25
# Github: https://github.com/i0gan/Squick
# Description: Build third party library

# 编译protobuf
#cd protobuf-21.6

third_party_path=`pwd`/../third_party
mongo_c_driver_install=$third_party_path/build/mongo-c-driver/install
build_type=Release

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


# build mongo-c-driver
cd $third_party_path
cd build/mongo-c-driver
cmake ../../mongo-c-driver -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=$build_type
cmake --build . -j $(nproc)
cmake --install . 
#cp install/* 
cp install/lib/*.so ../lib


# build mongo-cxx-driver
cd $third_party_path
cd build/mongo-cxx-driver
cmake ../../mongo-cxx-driver -DCMAKE_CXX_STANDARD=17 -DCMAKE_PREFIX_PATH=$mongo_c_driver_install -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=$build_type
cmake --build . -j 8
cmake --install .
cp -r install/include/bsoncxx/v_noabi/* ../include
cp -r install/include/mongocxx/v_noabi/* ../include
cp install/lib/*.so ../lib


# build libevent
cd $third_party_path
cd build/libevent
cmake  ../../libevent
make -j $(nproc)
mkdir -p ./install && make install DESTDIR=./install
cp -r install/usr/local/include/* ../include
#cp -r ./include/* ../include
cd ./lib
cp *.a ../../lib
cp *.so ../../lib



# build protobuf
cd $third_party_path
cd build/protobuf
cmake ../../protobuf -Dprotobuf_BUILD_TESTS=OFF -DBUILD_SHARED_LIBS=true
make -j $(nproc)
mkdir -p ./install && make install DESTDIR=./install
cp -r install/usr/local/include/* ../include
cp -r install/usr/local/bin/* ../bin
cp libprotobuf.so.3.21.6.0 ../lib/libprotobuf.so


# build lua
cd $third_party_path
cd lua
make linux
cp ./src/*.h ../build/include/
cp ./src/*.hpp ../build/include/
cp ./src/*.a ../build/lib
cp ./src/*.so ../build/lib



# build redis connector
cd $third_party_path
cd hiredis
make clean
make
cp *.a $third_party_path/build/lib
cp *.so $third_party_path/build/lib
cp *.h $third_party_path/build/include


# build mysql connector
cd $third_party_path
cd ./build/mysql-connector-cpp
cmake ../../mysql-connector-cpp
make -j $(nproc)
make -j $(nproc) # 再编译一次，protobuf内存不足的话会编译失败
mkdir -p ./install && make install DESTDIR=./install
cp -r install/usr/local/mysql/connector-c++-8.0.31/include/* ../include
cp libmysqlcppconn8.so ../lib/


# build zlib
cd $third_party_path
cd build/zlib
cmake  ../../zlib
make -j $(nproc)
cp *.a $third_party_path/build/lib
cp *.so $third_party_path/build/lib
cp *.h $third_party_path/build/include


# build navigation
cd $third_party_path
cd build/navigation
cmake  ../../recastnavigation
make -j $(nproc)
cp *.a $third_party_path/build/lib
cp *.so $third_party_path/build/lib
#cp *.h $third_party_path/build/include


cd $third_party_path
# fix change mode
chmod +x $third_party_path/build/lib/*.so
