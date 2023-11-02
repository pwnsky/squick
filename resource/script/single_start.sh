#!/bin/bash

#export LC_ALL="C"
#ulimit -c unlimited
#source /etc/profile

cd bin
# for linux
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
# for macos
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:`pwd`

./squick &
sleep 1

ps -A|grep squick
