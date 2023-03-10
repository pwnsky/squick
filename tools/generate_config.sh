#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Generate configuration files

config_path="../config"
struct_path="../src/squick/struct"
lua_proto_path=
client_config_path="../client"
excel_path='../resource/excel'
bash ./clean_config.sh
# 生成配置文件
bash ./proto2code.sh
mkdir -p $config_path/excel
mkdir -p $config_path/struct
mkdir -p $config_path/ini

./xlsx2need $excel_path $config_path
cp -a $config_path/excel/excel.h $struct_path

mkdir -p $client_config_path/ini
mkdir -p $client_config_path/excel
mkdir -p $client_config_path/struct
mkdir -p $client_config_path/lua

cp -a $config_path/ini $client_config_path
cp -a $config_path/struct $client_config_path
rm -rf $config_path/excel

# 生成Lua文件
python3 proto_enum_to_lua.py
python3 proto_to_lua_str.py
cp "../src/lua/proto/enum.lua" $client_config_path/lua/
