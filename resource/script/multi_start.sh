#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Start servers for devlopment debug

#export LC_ALL="C"
ulimit -c unlimited
#source /etc/profile
squick_bin_path=./bin
cd $squick_bin_path
# for linux
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
# for macos
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:`pwd`

chmod -R 777  squick

./squick type=master id=1 &
./squick type=world id=100 &
./squick type=world id=101 &
./squick type=db_proxy id=300 &
./squick type=db_proxy id=301 &
./squick type=login id=2 &

./squick type=lobby id=1000 &
./squick type=lobby id=1001 &
./squick type=lobby id=1002 &

./squick type=game id=3000 &
./squick type=game id=3001 &
./squick type=game_mgr id=2001 &

./squick type=proxy id=500 &
./squick type=proxy id=501 &
./squick type=proxy id=502 &

0.5
ps -A|grep squick
