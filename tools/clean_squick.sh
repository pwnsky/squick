#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Clean all generated files

rm -rf ../cache
rm -rf ../bin
rm -rf ../data
rm -rf ../client
rm -rf ../deploy
rm -rf ../src/squick/struct/*.pb.h
rm -rf ../src/squick/struct/*.pb.cc
rm -rf ../src/squick/struct/protocol_define.h
rm ./xlsx2need
