#!/bin/bash

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
