rem Author: i0gan
rem Email : l418894113@gmail.com
rem Date  : 2022-11-27
rem Github: https://github.com/i0gan/Squick
rem Description: Start all servers script

cd bin

start cmd /c " squick type=master id=1 area=0 ip=127.0.0.1 port=10001 web_port=8888"
start cmd /c " squick type=world id=100 area=0 ip=127.0.0.1 port=10101 master_ip=127.0.0.1 master_port=10001"
start cmd /c " squick type=db_proxy id=300 area=0 ip=127.0.0.1 port=10201 master_ip=127.0.0.1 master_port=10001"
start cmd /c " squick type=login id=2 area=0 ip=127.0.0.1 port=10301 web_port=80 master_ip=127.0.0.1 master_port=10001"
start cmd /c " squick type=lobby id=1000 area=0 ip=127.0.0.1 port=10401 master_ip=127.0.0.1 master_port=10001"
start cmd /c " squick type=lobby id=1001 area=0 ip=127.0.0.1 port=10402 master_ip=127.0.0.1 master_port=10001"
start cmd /c " squick type=proxy id=500 area=0 ip=127.0.0.1 port=10501 master_ip=127.0.0.1 master_port=10001"