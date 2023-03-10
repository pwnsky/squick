#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-12-19
# Github: https://github.com/i0gan/Squick
# Description: Start servers for devlopment test

#export LC_ALL="C"
#ulimit -c unlimited
#source /etc/profile
squick_bin_path=../bin
cd $squick_bin_path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib

#./squick plugin=gateway.xml server=gateway id=10
#./squick plugin=proxy.xml server=proxy id=5
./squick plugin=pvp.xml server=pvp id=11

sleep 1

ps -A|grep squick
