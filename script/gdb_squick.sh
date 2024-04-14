#!/bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2023-04-04
# Github: https://github.com/pwnsky/squick
# Description: Gdb to debug the squick

ulimit -c unlimited

cd ../bin
# for linux
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
# for macos
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:`pwd`

echo $SQUICK_ARGS
gdb ./squick
