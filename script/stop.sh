#!/bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/pwnsky/squick
# Description: Stop all nodes
cd $(dirname $0)

ps -A|grep squick
#pgrep squick | xargs kill -s 9
kill -2 $(ps -ef|grep squick |awk '$0 !~/grep/ {print $2}' |tr -s '\n' ' ')
sleep 3
# force kill
kill -9 $(ps -ef|grep squick |awk '$0 !~/grep/ {print $2}' |tr -s '\n' ' ')
