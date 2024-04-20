#!/bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-10-01
# Github: https://github.com/pwnsky/squick
# Description: Install development environment on linux

source common.sh
export LD_LIBRARY_PATH=`realpath  ../third_party/build/bin`:`realpath  ../third_party/build/lib`:`realpath ../third_party/build/protobuf`
echo $LD_LIBRARY_PATH

proto_bin='../third_party/build/bin/protoc'
proto_path="../src/proto"
cpp_out_path="../src/struct"
csharp_out_path='../client/proto/csharp'
lua_out_path="../client/proto/lua"

mkdir -p $csharp_out_path
mkdir -p $lua_out_path

# 生成Lua文件
cd proto
python3 gen_msgid.py
check_err
python3 proto_enum_to_lua.py
check_err
python3 proto_to_lua_str.py
cd ..

mkdir -p ../src/lua/proto
cp "../src/lua/proto/enum.lua" $lua_out_path


proto_files=$proto_path/*.proto
for f in $proto_files
do
    echo $f
    $proto_bin --cpp_out=$cpp_out_path  --proto_path=$proto_path $f
    $proto_bin --csharp_out=$csharp_out_path  --proto_path=$proto_path $f
    check_err
done

# remove nodes rpc files
rm $csharp_out_path/N*.cs

check_err
