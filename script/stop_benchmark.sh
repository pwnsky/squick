#!/bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2024-04-07
# Github: https://github.com/pwnsky/squick
# Description: Stop benchmark test

ps -A|grep sqkcli
#pgrep squick | xargs kill -s 9
kill -2 $(ps -ef|grep sqkcli |awk '$0 !~/grep/ {print $2}' |tr -s '\n' ' ')
pkill sqkcli
sleep 1
