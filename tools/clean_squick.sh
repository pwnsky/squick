#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/pwnsky/squick
# Description: Clean all generated files
cd $(dirname $0)
rm -rf ../cache
rm -rf ../bin
rm -rf ../data
rm -rf ../client
rm -rf ../deploy
rm -rf ../src/struct/*.pb.h
rm -rf ../src/struct/*.pb.cc
rm -rf ../src/struct/excel.h
