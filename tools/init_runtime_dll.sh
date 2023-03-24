#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Build deploy files

mkdir -p ../bin
cp ../third_party/build/lib/*.so ../bin
cp ../third_party/build/lib/libprotobuf.so ../bin/libprotobuf.so.32
cp ../third_party/build/lib/libmysqlcppconn8.so ../bin/libmysqlcppconn8.so.2
cp ../third_party/build/lib/libmongocxx.so ../bin/libmongocxx.so._noabi
cp ../third_party/build/lib/libbsoncxx.so ../bin/libbsoncxx.so._noabi
