#!/bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2024-04-08
# Github: https://github.com/pwnsky/squick
# Description: Start all nodes

./squick.sh type=master id=1 area=0 ip=127.0.0.1 port=10001 web_port=8888 &
sleep 0.5
./squick.sh type=world id=100 area=0 ip=127.0.0.1 port=10101 master_ip=127.0.0.1 master_port=10001 &
sleep 0.5
./squick.sh type=db_proxy id=300 area=0 ip=127.0.0.1 port=10201 master_ip=127.0.0.1 master_port=10001 &
sleep 0.5
./squick.sh type=login id=2 area=0 ip=127.0.0.1 port=10301 web_port=80 master_ip=127.0.0.1 master_port=10001 &
sleep 0.5
./squick.sh type=lobby id=1000 area=0 ip=127.0.0.1 port=10401 master_ip=127.0.0.1 master_port=10001 &
sleep 0.5
./squick.sh type=lobby id=1001 area=0 ip=127.0.0.1 port=10402 master_ip=127.0.0.1 master_port=10001 &
sleep 0.5
./squick.sh type=proxy id=500 area=0 ip=127.0.0.1 port=10501 master_ip=127.0.0.1 master_port=10001 &
