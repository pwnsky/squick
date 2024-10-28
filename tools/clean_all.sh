#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/pwnsky/squick
# Description: Clean all generated files
cd $(dirname $0)
source common.sh
third_party_path=`pwd`/../third_party

cd ..

rm -rf ./cache
rm -rf ./deploy/bin
rm -rf ./deploy/config/client/*
rm -rf ./deploy/config/struct/*
rm -rf ./deploy/config/proto/*

rm -rf ./bin
rm -rf ./tools/bin
rm -rf ./data

rm -rf ./deploy/*.log
rm -rf ./deploy/data/logs
rm -rf ./deploy/data/crash
rm -rf ./src/struct/*.cs
rm -rf ./src/struct/*.pb.h
rm -rf ./src/struct/*.pb.cc
rm -rf ./src/struct/excel.h
rm -rf ./third_party/build
rm -rf ./res
rm -rf ./pycli/proto

# clean third party
cd $third_party_path

rm -rf ./build
find . -name "*.o" | xargs  rm -rf
rm -rf ./lua/src/lua
rm -rf ./lua/src/luac
rm -rf ./lua/src/liblua.so
rm -rf ./lua/src/liblua.a

