#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Build deploy files

mkdir -p ../bin
cp ../third_party/build/lib/*.so ../bin
cd ../bin
ln -s libprotobuf.so libprotobuf.so.32
ln -s libmysqlcppconn8.so libmysqlcppconn8.so.2
ln -s libmongocxx.so libmongocxx.so._noabi
ln -s libbsoncxx.so libbsoncxx.so._noabi
ln -s libmongoc-1.0.so libmongoc-1.0.so.0
ln -s libbson-1.0.so libbson-1.0.so.0
ln -s libhiredis.so libhiredis.so.1.1.0
ln -s libredis++.so libredis++.so.1
