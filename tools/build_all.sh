#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/pwnsky/squick
# Description: build script for squick
cd $(dirname $0)
bash ./clean_all.sh
bash ./build_third_party.sh

cur_path=`pwd`
# build export tools
cd $cur_path/../res/ToolsSrc/sqkctl
bash ./build.sh

# build project
cd $cur_path
bash ./build.sh
