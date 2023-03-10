#!/bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-10-01
# Github: https://github.com/i0gan/Squick
# Description: Install development environment on linux

proto_bin='../third_party/build/protobuf/protoc'
proto_path="../src/proto"
cpp_out_path="../src/squick/struct"
csharp_out_path='../client/proto'

mkdir -p $csharp_out_path
proto_files=$proto_path/*
for f in $proto_files
do
    echo $f
    $proto_bin --cpp_out=$cpp_out_path  --proto_path=$proto_path $f
    $proto_bin --csharp_out=$csharp_out_path  --proto_path=$proto_path $f
done
