#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Clean all generated files
cd ..

rm -rf ./cache
rm -rf ./deploy/bin
rm -rf ./deploy/config/client/*
rm -rf ./deploy/config/struct/*
rm -rf ./deploy/config/proto/*

rm -rf ./bin
rm -rf ./tools/config_generator
rm -rf ./data

#rm -rf ./deploy/config/ini/*
rm -rf ./deploy/*.log
rm -rf ./deploy/data/logs
rm -rf ./deploy/data/www
rm -rf ./src/squick/struct/*.cs
rm -rf ./src/squick/struct/*.pb.h
rm -rf ./src/squick/struct/*.pb.cc
rm -rf ./src/squick/struct/protocol_define.h
rm -rf ./third_party/build

#cd ./src/www/admin
#bash clean.sh
#cd ..
