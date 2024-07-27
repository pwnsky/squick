#!/bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2024-04-08
# Github: https://github.com/pwnsky/squick
# Description: Start all nodes
cd $(dirname $0)

./squick.sh type=master id=1 area=0 ip=127.0.0.1 port=10001 http_port=50000 logshow=0 &
sleep 0.5
./squick.sh type=backstage id=10 area=0 ip=127.0.0.1 port=10010 http_port=8888 master_ip=127.0.0.1 master_port=10001 logshow=0 &
sleep 0.5
./squick.sh type=world id=100 area=0 ip=127.0.0.1 port=10101 master_ip=127.0.0.1 master_port=10001 logshow=0 &
sleep 0.5
./squick.sh type=db_proxy id=300 area=0 ip=127.0.0.1 port=10201 master_ip=127.0.0.1 master_port=10001 logshow=0 &
sleep 0.5
./squick.sh type=web id=2 area=0 ip=127.0.0.1 port=10301 http_port=8088 master_ip=127.0.0.1 master_port=10001 logshow=0 &
sleep 0.5
./squick.sh type=player id=1000 area=0 ip=127.0.0.1 port=10401 master_ip=127.0.0.1 master_port=10001 logshow=0 &
sleep 0.5
./squick.sh type=player id=1001 area=0 ip=127.0.0.1 port=10402 master_ip=127.0.0.1 master_port=10001 logshow=0 &
sleep 0.5
./squick.sh type=proxy id=500 area=0 ip=127.0.0.1 port=10501 ws_port=10502 master_ip=127.0.0.1 master_port=10001 logshow=0 &
