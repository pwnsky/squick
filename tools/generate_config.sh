#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/pwnsky/squick
# Description: Generate configuration files

config_path="../config"
struct_path="../src/struct"
client_config_path="../client"
excel_path='../resource/excel'


bash ./clean_config.sh
# 生成配置文件
mkdir -p $config_path/excel
mkdir -p $config_path/struct
mkdir -p $config_path/ini

./bin/sqkctl excel $excel_path $config_path
cp -a $config_path/excel/excel.h $struct_path

mkdir -p $client_config_path/ini
mkdir -p $client_config_path/excel
mkdir -p $client_config_path/struct

cp -a $config_path/ini $client_config_path
cp -a $config_path/struct $client_config_path
cp -a $config_path/excel $client_config_path

rm -rf $config_path/excel