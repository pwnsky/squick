#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: 生成Linux部署文件

echo "Generating linux deploy files"
bash init_runtime_dll.sh

cd ..
rm -rf deploy
mkdir deploy
mkdir deploy/src

cp -r bin deploy
cp -r config deploy
cp -r resource/script/*.sh deploy
cp -r src/proto deploy/src
cp -r src/lua deploy/src

echo "Generated all files!"

#cd deploy/bin
