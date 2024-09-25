#! /bin/bash

nums=1000
sleep_time=0.01
kill_time=20
if [ "$1" != "" ];then
nums=$1
fi

for ((i=1; i<=$nums; i++))
do
        timeout --foreground $kill_time python main.py >> batch_run.log &
        sleep $sleep_time
done

echo "Run benchmark with $nums process"
