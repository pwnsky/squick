#!/bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2024-4-8
# Github: https://github.com/pwnsky/squick
# Description: Execute sqkcli
cd $(dirname $0)

ulimit -c unlimited

cd ../bin
# for linux
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
# for macos
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:`pwd`

echo $SQUICK_ARGS
if [ -z $SQUICK_ARGS ];then
        ./sqkcli $@
else
        ./sqkcli $SQUICK_ARGS
fi
