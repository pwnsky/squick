#!/bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2024-04-08
# Github: https://github.com/pwnsky/squick
# Description: Execute squick
cd $(dirname $0)

ulimit -c unlimited

bash ./gen_env_config.sh

cd ../bin
# for linux
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
# for macos
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:`pwd`

echo $SQUICK_MEMORY_CHECK
if [ "1" == "$SQUICK_MEMORY_CHECK" ];then
        valgrind --leak-check=full ./squick $@
        exit
fi

echo $SQUICK_ARGS
if [ -z $SQUICK_ARGS ];then
        ./squick $@
else
        ./squick $SQUICK_ARGS
fi
