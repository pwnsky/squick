#!/bin/bash

#export LC_ALL="C"
#ulimit -c unlimited
#source /etc/profile

cd bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
#echo $LD_LIBRARY_PATH

./squick &
sleep 1
./www &
sleep 1

ps -A|grep squick
