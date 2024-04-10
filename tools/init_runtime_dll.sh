#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Build deploy files

mkdir -p ../bin
mkdir -p ../data/logs
mkdir -p ../data/crash

sys=`uname -s`
if [ $sys == "Darwin" ];then
    echo "Mac runtime init"
    cp ../third_party/build/lib/*.dylib ../bin
    cd ../bin
    ln -s libmysqlcppconn8.dylib libmysqlcppconn8.2.dylib
else
    echo "Linux runtime init"
    # for linux
    cp ../third_party/build/lib/*.so ../bin
    cd ../bin
    ln -sf libprotobuf.so libprotobuf.so.32
    ln -sf libmysqlcppconn8.so libmysqlcppconn8.so.2
    ln -sf libmongocxx.so libmongocxx.so._noabi
    ln -sf libbsoncxx.so libbsoncxx.so._noabi
    ln -sf libmongoc-1.0.so libmongoc-1.0.so.0
    ln -sf libbson-1.0.so libbson-1.0.so.0
    ln -sf libhiredis.so libhiredis.so.1.1.0
    ln -sf libredis++.so libredis++.so.1
fi


