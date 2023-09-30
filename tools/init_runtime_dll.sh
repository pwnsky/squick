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
cp ../third_party/build/lib/libmongoc-1.0.so ../bin/libmongoc-1.0.so.0
cp ../third_party/build/lib/libbson-1.0.so ../bin/libbson-1.0.so.0
cp ../third_party/build/lib/libhiredis.so ../bin/libhiredis.so.1.1.0
cp ../third_party/build/lib/libredis++.so ../bin/libredis++.so.1
