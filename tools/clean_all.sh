#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Clean all generated files
source common.sh

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
rm -rf ./deploy/data/www
rm -rf ./src/struct/*.cs
rm -rf ./src/struct/*.pb.h
rm -rf ./src/struct/*.pb.cc
rm -rf ./src/struct/protocol_define.h
rm -rf ./third_party/build

