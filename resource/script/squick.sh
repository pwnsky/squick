#!/bin/bash

ulimit -c unlimited

cd bin
# for linux
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
# for macos
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:`pwd`

./squick $@
