#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Start servers for devlopment debug

#export LC_ALL="C"
#ulimit -c unlimited
#source /etc/profile
squick_bin_path=./bin
cd $squick_bin_path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
#echo $LD_LIBRARY_PATH

chmod -R 777  squick

./squick plugin=master.xml server=master id=1 &
./squick plugin=db_proxy.xml server=db_proxy id=4 &
./squick plugin=world.xml server=world id=2 &
./squick plugin=login.xml server=login id=3 &
./squick plugin=gateway.xml server=gateway id=5 &

./squick plugin=game.xml server=game id=10 &
./squick plugin=game.xml server=game id=11 &

./squick plugin=gameplay_manager.xml server=gameplay_manager id=20 &

./squick plugin=proxy.xml server=proxy id=6 &

0.5
ps -A|grep squick
