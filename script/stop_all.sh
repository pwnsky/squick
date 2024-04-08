#!/bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-11-27
# Github: https://github.com/i0gan/Squick
# Description: Stop servers for devlopment debug

ps -A|grep squick
#pgrep squick | xargs kill -s 9
kill -2 $(ps -ef|grep squick |awk '$0 !~/grep/ {print $2}' |tr -s '\n' ' ')
sleep 1
