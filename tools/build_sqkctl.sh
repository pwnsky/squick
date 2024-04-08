#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/pwnsky/squick
# Description: Build sqkctl tools

source common.sh

echo "build_path: " $build_path
echo "project_path: " $project_path
# 编译工具
cd ${project_path}
mkdir -p "${build_path}/sqkctl"
cd "${build_path}/sqkctl"
pwd

log_info "Start to build sqkctl tools"

cmake ${project_path}/src/tools/sqkctl
time make -j $(nproc)
