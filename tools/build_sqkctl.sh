#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Build sqkctl tools

source source/build.sh

echo $build_path
# 编译工具
cd ${project_path}
mkdir -p "${build_path}/tools"
cd "${build_path}/tools"
cmake ${project_path}/src/tools/sqkctl
make -j $(nproc)