#!/bin/bash

./squick.sh -d type=master id=1 area=0 ip=127.0.0.1 port=10001 web_port=8888
./squick.sh -d type=world id=100 area=0 ip=127.0.0.1 port=10101 master_ip=127.0.0.1 master_port=10001
./squick.sh -d type=db_proxy id=300 area=0 ip=127.0.0.1 port=10201 master_ip=127.0.0.1 master_port=10001
./squick.sh -d type=login id=2 area=0 ip=127.0.0.1 port=10301 web_port=80 master_ip=127.0.0.1 master_port=10001
./squick.sh -d type=lobby id=1000 area=0 ip=127.0.0.1 port=10401 master_ip=127.0.0.1 master_port=10001
./squick.sh -d type=lobby id=1001 area=0 ip=127.0.0.1 port=10402 master_ip=127.0.0.1 master_port=10001
./squick.sh -d type=proxy id=500 area=0 ip=127.0.0.1 port=10501 master_ip=127.0.0.1 master_port=10001
