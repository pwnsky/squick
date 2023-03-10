#!/bin/bash

third_party_path=`pwd`/../third_party
cd $third_party_path

rm -rf ./build
find . -name "*.o" | xargs  rm -rf
rm -rf ./lua/src/lua
rm -rf ./lua/src/luac
rm -rf ./lua/src/liblua.so
rm -rf ./lua/src/liblua.a