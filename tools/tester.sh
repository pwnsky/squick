#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Test all servers

export LC_ALL="C"

ulimit -c unlimited
source /etc/profile


cd server
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
#echo $LD_LIBRARY_PATH

chmod -R 777  squick
./squick plugin=plugin.xml server=Masterserver id=3

ps -A|grep squick
