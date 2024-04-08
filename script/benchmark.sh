#! /bin/bash
# Author: i0gan
# Email : l418894113@gmail.com
# Date  : 2022-05-07
# Github: https://github.com/pwnsky/squick
# Description: start 100 testers to test server
ulimit -c unlimited

cd ../bin
# for linux
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/lib
# for macos
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:`pwd`
nums=100
if [ "$1" != "" ];then
nums=$1
fi

for ((i=1; i<=$nums; i++))
do
	./sqkcli type=sqkcli test=proxy ip=127.0.0.1 port=10501 hide=true id=1 > /dev/null &
done

echo "Run benchmark with $nums process"
